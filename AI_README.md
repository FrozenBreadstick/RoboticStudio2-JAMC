AI_README.md
AprilTag Piano Detector

A ROS 2 node that detects piano keys using YOLO segmentation and correlates them with AprilTag markers via an Intel realsense camera.

Dependencies 
System Requirements
1 Ubuntu 22.04
2 ROS 2 Humble 
3 CUDA capable GPU (recomended)

ROS 2 Packages
# RealSense camera driver
sudo apt install ros-humble-realsense2-camera

# AprilTag detection
sudo apt install ros-humble-apriltag

Python Packages 
pip install torch torchvision --index-url https://download.pytorch.org/whl/cu121
pip install ultralytics opencv-python apriltag cv-bridge numpy

Important Note: If CUDA is unavailable, the node will fall back to CPU automatically, but inference will be significantly slower. 
