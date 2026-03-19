# How to setup Robot to run our code
## UR Driver Setup
```bash
sudo apt-get install ros-humble-ur
```

## Running the Driver
```bash
# Replace ur5e with one of ur3, ur5, ur10, ur3e, ur5e, ur7e, ur10e, ur12e, ur16e, ur8long, ur15, ur18, ur20, ur30
# Replace the IP address with the IP address of your actual robot / URSim
ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur5e robot_ip:=192.168.56.101
```

## Extracting Robot Calibration
```bash
# Replace the IP address with the IP address of your actual robot / URSim
# Calibration extraction will be saved to the target_filename
ros2 launch ur_calibration calibration_correction.launch.py robot_ip:=<robot_ip> target_filename:="${HOME}/my_robot_calibration.yaml"
```