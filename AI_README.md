# How to use the AprilTag Piano Detector

A ROS 2 node that detects piano keys using YOLO segmentation and correlates them with AprilTag markers via an Intel RealSense camera.

## 1. Installation

### Dependencies

**System Requirements**
1. Ubuntu 22.04
2. ROS 2 Humble
3. CUDA capable GPU (recommended)

**ROS 2 Packages**

RealSense camera driver:
```bash
sudo apt install ros-humble-realsense2-camera
```

AprilTag detection:
```bash
sudo apt install ros-humble-apriltag
```

**Python Packages**
```bash
pip install torch torchvision --index-url https://download.pytorch.org/whl/cu121
pip install ultralytics opencv-python apriltag cv-bridge numpy
```

> **Important Note:** If CUDA is unavailable, the node will fall back to CPU automatically, but inference will be significantly slower.

## 2. Camera Setup

### Connecting the RealSense Camera

1. Plug the Intel RealSense camera into a USB 3.0 port.
2. Verify the device is detected by the system:
```bash
   rs-enumerate-devices
```
3. Confirm the ROS 2 driver can see the camera:
```bash
   ros2 launch realsense2_camera rs_launch.py
```
4. In a second terminal, verify the colour stream is publishing:
```bash
   ros2 topic list | grep camera
```

## 3. Software Setup

### Place the YOLO Weights

Ensure the trained weights file is located at the correct path relative to your workspace root before running the node.

### Build the Workspace
```bash
cd ~/your_ros2_ws
colcon build --symlink-install
source install/setup.bash
```

### Launch the Camera
```bash
ros2 launch realsense2_camera rs_launch.py
```

### Run the Detection Node
```bash
source install/setup.bash
ros2 run perception apriltag_piano_detector
```

> **Note:** To shut down the OpenCV window, press `q`.

### Verify Output
```bash
ros2 topic echo /piano_keys
ros2 topic echo /debug_target
```

## 4. Node Interface

The `AprilTagPianoDetector` node exposes no public class members and is instantiated by calling `main()`. See below for all ROS 2 interfaces exposed at runtime.

### Interface List

#### Subscribers

| Interface Name | Type | Description |
|----------------|------|-------------|
| `subscription` | `sensor_msgs/Image` | Raw colour stream from the RealSense camera. Triggers the full detection pipeline on every frame. |

#### Publishers

| Interface Name | Type | Description |
|----------------|------|-------------|
| `publisher_tag` | `geometry_msgs/Point` | Normalised (x, y) offset of the detected AprilTag from the frame centre. Range `[-1.0, 1.0]`. Positive x = right, positive y = up. z is unused. |
| `publisher_keys` | `geometry_msgs/PoseArray` | Left-to-right ordered array of detected piano key centroids. Each `pose.position.x/y` is the adjusted centroid in pixel coordinates. |

## 5. Configuration

The following constants at the top of the code can be adjusted without modifying any other part of the code.

| Constant | Default | Description |
|----------|---------|-------------|
| `YOLO_IMG_SIZE` | `640` | Input resolution fed to YOLO (pixels, square crop) |
| `YOLO_SKIP` | `1` | Run YOLO every N frames. Increase to reduce CPU/GPU load |
| `YOLO_HALF` | `True` | FP16 half-precision inference on CUDA (faster, slightly less accurate) |
| `DISPLAY_SCALE` | `1.0` | Scale factor applied to the OpenCV preview window |
| `WHITE_KEY_OFFSET` | `(0, 30)` | Pixel offset (dx, dy) added to white key centroids before publishing |
| `BLACK_KEY_OFFSET` | `(0, 10)` | Pixel offset (dx, dy) added to black key centroids before publishing |