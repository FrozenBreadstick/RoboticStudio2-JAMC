#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSHistoryPolicy
from sensor_msgs.msg import Image
from geometry_msgs.msg import Point, Pose, PoseArray
from std_msgs.msg import String
from cv_bridge import CvBridge

import cv2
import apriltag
import numpy as np
import json
import torch
from ultralytics import YOLO


YOLO_IMG_SIZE  = 640    # Full YOLO resolution
YOLO_SKIP      = 1      # Run YOLO every frame
YOLO_HALF      = True
DISPLAY_SCALE  = 1.0    # Full resolution preview

#                      ( X offset,  Y offset )
WHITE_KEY_OFFSET = (0, 30)
BLACK_KEY_OFFSET = (0, 10)



class AprilTagPianoDetector(Node):

    def __init__(self):
        super().__init__('apriltag_piano_detector')

        qos = QoSProfile(
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=QoSReliabilityPolicy.BEST_EFFORT
        )

        self.subscription = self.create_subscription(
            Image,
            '/camera/camera/color/image_raw',
            self.image_callback,
            qos
        )

        self.publisher_tag  = self.create_publisher(Point,     '/debug_target', 10)
        self.publisher_keys = self.create_publisher(PoseArray, '/piano_keys',   10)

        if not torch.cuda.is_available():
            self.get_logger().warn(
                "CUDA not found! Check your PyTorch install:\n"
                "  pip install torch torchvision --index-url "
                "https://download.pytorch.org/whl/cu121"
            )
        self.device = 'cuda' if torch.cuda.is_available() else 'cpu'
        self.yolo_model = YOLO('src/perception/best.pt')
        self.yolo_model.to(self.device)
        if YOLO_HALF and self.device == 'cuda':
            self.yolo_model.model.half()
        gpu_name = torch.cuda.get_device_name(0) if self.device == 'cuda' else 'N/A'
        self.get_logger().info(
            f"YOLO device : {self.device.upper()}  ({gpu_name})\n"
            f"  imgsz={YOLO_IMG_SIZE}  half={YOLO_HALF and self.device=='cuda'}  skip={YOLO_SKIP}"
        )

        self.tag_detector = apriltag.Detector()

        self.bridge = CvBridge()

        self._frame_count  = 0
        self.ordered_keys  = []

        self._fps_t0    = None
        self._fps_count = 0
        self._fps       = 0.0

        cv2.namedWindow("AprilTag + Piano Keys", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("AprilTag + Piano Keys", 960, 540)

        self.get_logger().info(
            f"Detector ready  |  YOLO imgsz={YOLO_IMG_SIZE}  "
            f"skip={YOLO_SKIP}  device={self.device.upper()}\n"
            f"  Waiting on /camera/camera/color/image_raw\n"
            f"  Launch camera first:  ros2 launch realsense2_camera rs_launch.py"
        )

    def image_callback(self, msg: Image):
        try:
            bgr = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        except Exception as e:
            self.get_logger().error(f"Image conversion failed: {e}")
            return

        self._frame_count += 1
        h, w = bgr.shape[:2]
        center_x = w / 2.0
        center_y = h / 2.0

        if self._frame_count % YOLO_SKIP == 0:
            self.ordered_keys = self._detect_piano_keys(bgr)
            self._publish_keys()

        display = bgr.copy()
        self._draw_piano_keys(display, self.ordered_keys)

        gray    = cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY)
        results = self.tag_detector.detect(gray)

        # ── Frame centre marker (drawn once, behind all tag lines) ────────────
        frame_cx = int(center_x)
        frame_cy = int(center_y)
        cv2.drawMarker(
            display, (frame_cx, frame_cy),
            color=(0, 255, 255),
            markerType=cv2.MARKER_CROSS,
            markerSize=20,
            thickness=2
        )
        # ──────────────────────────────────────────────────────────────────────

        for r in results:
            (ptA, ptB, ptC, ptD) = r.corners
            ptA = tuple(int(x) for x in ptA)
            ptB = tuple(int(x) for x in ptB)
            ptC = tuple(int(x) for x in ptC)
            ptD = tuple(int(x) for x in ptD)

            cv2.line(display, ptA, ptB, (0, 255, 0), 2)
            cv2.line(display, ptB, ptC, (0, 255, 0), 2)
            cv2.line(display, ptC, ptD, (0, 255, 0), 2)
            cv2.line(display, ptD, ptA, (0, 255, 0), 2)

            cX, cY = int(r.center[0]), int(r.center[1])
            cv2.circle(display, (cX, cY), 5, (255, 0, 0), -1)
            cv2.putText(display, f"TAG {r.tag_id}", (cX, cY - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

            # ── Line from frame centre → tag centre ───────────────────────────
            cv2.line(display, (frame_cx, frame_cy), (cX, cY), (0, 255, 255), 2)

            # Distance in pixels
            dx   = cX - frame_cx
            dy   = cY - frame_cy
            dist = (dx ** 2 + dy ** 2) ** 0.5

            # Place the label at the midpoint of the line, offset slightly so
            # it does not sit directly on top of the line itself.
            label_x = (frame_cx + cX) // 2 + 8
            label_y = (frame_cy + cY) // 2 - 8
            cv2.putText(
                display,
                f"{dist:.1f}px",
                (label_x, label_y),
                cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 255), 2
            )
            # ──────────────────────────────────────────────────────────────────

            offset_x = (cX - center_x) / center_x
            offset_y = -((cY - center_y) / center_y)

            nearest = self._find_nearest_key(cX, cY)
            if nearest is not None:
                cv2.putText(
                    display,
                    f"Key #{nearest['key_index']}",
                    (cX + 10, cY + 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2
                )

            pt   = Point()
            pt.x = float(offset_x)
            pt.y = float(offset_y)
            pt.z = 0.0
            self.publisher_tag.publish(pt)

        import time
        now = time.monotonic()
        if self._fps_t0 is None:
            self._fps_t0 = now
        self._fps_count += 1
        elapsed = now - self._fps_t0
        if elapsed >= 1.0:
            self._fps       = self._fps_count / elapsed
            self._fps_count = 0
            self._fps_t0    = now

        cv2.putText(display, f"FPS: {self._fps:.1f}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
        cv2.putText(display, f"Keys: {len(self.ordered_keys)}",
                    (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 255), 2)
        cv2.putText(display, f"Device: {self.device.upper()}",
                    (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (180, 180, 180), 2)

        if DISPLAY_SCALE != 1.0:
            dw = int(w * DISPLAY_SCALE)
            dh = int(h * DISPLAY_SCALE)
            display = cv2.resize(display, (dw, dh), interpolation=cv2.INTER_LINEAR)

        cv2.imshow("AprilTag + Piano Keys", display)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            self.get_logger().info("'q' pressed – shutting down…")
            rclpy.shutdown()


    def _detect_piano_keys(self, bgr_image: np.ndarray) -> list:
        yolo_results = self.yolo_model(
            bgr_image,
            imgsz=YOLO_IMG_SIZE,
            half=(YOLO_HALF and self.device == 'cuda'),
            verbose=False,
            device=self.device
        )

        raw_keys = []
        for result in yolo_results:
            names  = result.names
            boxes  = result.boxes
            masks  = result.masks

            if masks is None:
                for box in boxes:
                    x1, y1, x2, y2 = box.xyxy[0].tolist()
                    conf  = float(box.conf[0])
                    label = names.get(int(box.cls[0]), str(int(box.cls[0])))
                    off_x, off_y = WHITE_KEY_OFFSET if label == "key_B" else BLACK_KEY_OFFSET
                    raw_keys.append({
                        "label":      label,
                        "bbox":       [x1, y1, x2, y2],
                        "contour":    [],
                        "mid_x":      (x1 + x2) / 2.0 + off_x,
                        "mid_y":      (y1 + y2) / 2.0 + off_y,
                        "confidence": conf,
                    })
                continue

            for box, mask in zip(boxes, masks.xy):
                x1, y1, x2, y2 = box.xyxy[0].tolist()
                conf  = float(box.conf[0])
                label = names.get(int(box.cls[0]), str(int(box.cls[0])))
                off_x, off_y = WHITE_KEY_OFFSET if label == "key_B" else BLACK_KEY_OFFSET

                contour = mask.astype(np.float32)
                if len(contour) >= 3:
                    M = cv2.moments(contour)
                    if M["m00"] != 0:
                        cx = M["m10"] / M["m00"]
                        cy = M["m01"] / M["m00"]
                    else:
                        cx = (x1 + x2) / 2.0
                        cy = (y1 + y2) / 2.0
                else:
                    cx = (x1 + x2) / 2.0
                    cy = (y1 + y2) / 2.0

                raw_keys.append({
                    "label":      label,
                    "bbox":       [x1, y1, x2, y2],
                    "contour":    contour.tolist(),
                    "mid_x":      cx + off_x,
                    "mid_y":      cy + off_y,
                    "confidence": conf,
                })

        raw_keys.sort(key=lambda k: k["bbox"][0])
        return [{"key_index": i, **k} for i, k in enumerate(raw_keys)]

    def _draw_piano_keys(self, bgr_image: np.ndarray, keys: list) -> None:
        overlay = bgr_image.copy()
        for k in keys:
            contour = k["contour"]
            label   = k["label"]
            if not contour:
                continue
            pts = np.array(contour, dtype=np.int32).reshape((-1, 1, 2))
            if label == "key_B":
                fill = (0, 200, 200)
            elif label == "key_W":
                fill = (180, 0, 180)
            else:
                fill = (0, 160, 200)
            cv2.fillPoly(overlay, [pts], fill)
        cv2.addWeighted(overlay, 0.30, bgr_image, 0.70, 0, bgr_image)

        for k in keys:
            contour = k["contour"]
            label   = k["label"]
            conf    = k["confidence"]
            idx     = k["key_index"]
            mx, my  = int(k["mid_x"]), int(k["mid_y"])
            x1      = int(k["bbox"][0])
            y1      = int(k["bbox"][1])

            if label == "key_B":
                colour = (0, 255, 255)
            elif label == "key_W":
                colour = (255, 0, 255)
            else:
                colour = (0, 215, 255)

            if contour:
                pts = np.array(contour, dtype=np.int32).reshape((-1, 1, 2))
                cv2.polylines(bgr_image, [pts], isClosed=True, color=colour, thickness=2)

            arm = 8
            cv2.line(bgr_image, (mx - arm, my), (mx + arm, my), colour, 2)
            cv2.line(bgr_image, (mx, my - arm), (mx, my + arm), colour, 2)
            cv2.circle(bgr_image, (mx, my), 3, (255, 255, 255), -1)

            cv2.putText(bgr_image, f"({mx},{my})",
                        (mx - 22, my + 18),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.35, colour, 1)

            cv2.putText(bgr_image, f"#{idx}",
                        (x1, y1 - 8),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, colour, 2)

            cv2.putText(bgr_image, f"{label} {conf:.2f}",
                        (x1 + 2, y1 + 18),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.42, colour, 1)

    def _publish_keys(self) -> None:
        msg = PoseArray()
        for k in self.ordered_keys:
            pose = Pose()
            pose.position.x = round(k["mid_x"], 1)
            pose.position.y = round(k["mid_y"], 1)
            pose.position.z = 0.0
            msg.poses.append(pose)
        self.publisher_keys.publish(msg)

    def _find_nearest_key(self, px: float, py: float) -> dict | None:
        if not self.ordered_keys:
            return None
        return min(
            self.ordered_keys,
            key=lambda k: (k["mid_x"] - px) ** 2 + (k["mid_y"] - py) ** 2
        )


def main(args=None):
    rclpy.init(args=args)
    node = AprilTagPianoDetector()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()