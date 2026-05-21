#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSHistoryPolicy
from sensor_msgs.msg import Image
from geometry_msgs.msg import Pose, PoseArray
from cv_bridge import CvBridge

import cv2
import numpy as np
import torch
from ultralytics import YOLO

YOLO_IMG_SIZE  = 640
YOLO_SKIP      = 1
YOLO_HALF      = True
YOLO_CONF      = 0.2  # Lowered to 0.2 to help catch dots

# (X offset, Y offset) pixel offset added to the raw key centroid
WHITE_KEY_OFFSET = (0, 30)
BLACK_KEY_OFFSET = (0, 10)

class YoloPianoDetector(Node):
    def __init__(self):
        super().__init__('yolo_piano_detector')

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

        self.publisher_keys = self.create_publisher(PoseArray, '/piano_keys', 10)

        # Initialize Device and YOLO Model
        self.device = 'cuda' if torch.cuda.is_available() else 'cpu'
        self.yolo_model = YOLO('src/RoboticStudio2-JAMC/src/perception/best2.pt')
        self.yolo_model.to(self.device)
        
        if YOLO_HALF and self.device == 'cuda':
            self.yolo_model.model.half()

        self.bridge = CvBridge()
        self._frame_count = 0

        # Dedicated variables for the dots (now storing dictionaries)
        self.blue_dot = None
        self.red_dot = None
        self.green_dot = None

        cv2.namedWindow("YOLO Detection", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("YOLO Detection", 960, 540)
        self.get_logger().info("Detector ready. Waiting for camera feed...")

    def image_callback(self, msg: Image):
        try:
            bgr = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        except Exception as e:
            self.get_logger().error(f"Image conversion failed: {e}")
            return

        self._frame_count += 1
        
        if self._frame_count % YOLO_SKIP == 0:
            self._process_detections(bgr)

        # Check for quit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            self.get_logger().info("'q' pressed – shutting down…")
            rclpy.shutdown()

    def _process_detections(self, bgr_image: np.ndarray):
        results = self.yolo_model(
            bgr_image,
            imgsz=YOLO_IMG_SIZE,
            half=(YOLO_HALF and self.device == 'cuda'),
            verbose=False,
            device=self.device,
            conf=YOLO_CONF
        )

        piano_keys = []
        
        # Reset dot positions for the current frame
        self.blue_dot = None
        self.red_dot = None
        self.green_dot = None

        for result in results:
            names = result.names
            boxes = result.boxes
            masks = result.masks

            for i, box in enumerate(boxes):
                x1, y1, x2, y2 = box.xyxy[0].tolist()
                label = names.get(int(box.cls[0]), str(int(box.cls[0])))
                conf = float(box.conf[0])

                # Default centroid from bounding box
                cx = (x1 + x2) / 2.0
                cy = (y1 + y2) / 2.0
                contour_pts = None

                # Use mask for more accurate centroid if segmentation is available
                if masks is not None and len(masks.xy) > i:
                    contour = masks.xy[i].astype(np.float32)
                    if len(contour) >= 3:
                        contour_pts = np.array(contour, dtype=np.int32).reshape((-1, 1, 2))
                        M = cv2.moments(contour)
                        if M["m00"] != 0:
                            cx = M["m10"] / M["m00"]
                            cy = M["m01"] / M["m00"]

                bbox_coords = (int(x1), int(y1), int(x2), int(y2))
                
                # Debug print to terminal
                print(f"Found: {label} ({conf:.2f})")

                # Categorize the detection using exact label matches
                if label == "blue":
                    self.blue_dot = {"pos": (cx, cy), "bbox": bbox_coords, "contour": contour_pts}
                elif label == "Red":
                    self.red_dot = {"pos": (cx, cy), "bbox": bbox_coords, "contour": contour_pts}
                elif label == "Green":
                    self.green_dot = {"pos": (cx, cy), "bbox": bbox_coords, "contour": contour_pts}
                elif label in ["Key-W", "Key-B"]:
                    off_x, off_y = WHITE_KEY_OFFSET if label == "Key-W" else BLACK_KEY_OFFSET
                    piano_keys.append({
                        "label": label,
                        "mid_x": cx + off_x,
                        "mid_y": cy + off_y,
                        "bbox": bbox_coords,
                        "contour": contour_pts
                    })

        # Sort the keys from left-to-right based on their x-coordinate
        piano_keys.sort(key=lambda k: k["mid_x"])

        # Execute actions based on detections
        self._publish_keys(piano_keys)
        self._draw_preview(bgr_image, piano_keys)

    def _publish_keys(self, keys: list):
        msg = PoseArray()
        
        # Append all sorted keys to the pose array
        for k in keys:
            pose = Pose()
            pose.position.x = float(k["mid_x"])
            pose.position.y = float(k["mid_y"])
            pose.position.z = 0.0
            msg.poses.append(pose)

        self.publisher_keys.publish(msg)

    def _draw_preview(self, img: np.ndarray, keys: list):
        # Draw Keys
        for i, k in enumerate(keys):
            mx, my = int(k["mid_x"]), int(k["mid_y"])
            x1, y1, x2, y2 = k["bbox"]
            box_color = (200, 200, 200) if k["label"] == "Key-W" else (50, 50, 50)

            # Draw Bounding Box
            cv2.rectangle(img, (x1, y1), (x2, y2), box_color, 2)
            # Draw Segmentation Mask Outline
            if k["contour"] is not None:
                cv2.polylines(img, [k["contour"]], isClosed=True, color=(255, 0, 255), thickness=2)

            cv2.circle(img, (mx, my), 5, (255, 0, 255), -1)
            cv2.putText(img, f"#{i}", (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, box_color, 2)

        # Draw Dots helper function
        def draw_dot(dot_dict, color, name):
            if dot_dict:
                bx, by = int(dot_dict["pos"][0]), int(dot_dict["pos"][1])
                x1, y1, x2, y2 = dot_dict["bbox"]
                
                # Draw Box
                cv2.rectangle(img, (x1, y1), (x2, y2), color, 2)
                # Draw Mask Outline
                if dot_dict["contour"] is not None:
                    cv2.polylines(img, [dot_dict["contour"]], isClosed=True, color=color, thickness=2)
                    
                cv2.circle(img, (bx, by), 8, color, -1)
                cv2.putText(img, name, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

        draw_dot(self.blue_dot, (255, 0, 0), "Blue")
        draw_dot(self.red_dot, (0, 0, 255), "Red")
        draw_dot(self.green_dot, (0, 255, 0), "Green")

        cv2.imshow("YOLO Detection", img)


def main(args=None):
    rclpy.init(args=args)
    node = YoloPianoDetector()
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