# How to setup Robot to run our code
## UR Driver Setup
```bash
sudo apt-get install ros-humble-ur
```

## Motion Planner Installation
```bash
sudo apt install ros-humble-moveit
sudo apt install ros-humble-ros2-control ros-humble-ros2-controllers
```

## Running the Driver
```bash
# Replace the IP address with the IP address of your actual robot / URSim
ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3e robot_ip:=192.168.0.191 launch_rviz:=false
```

## Running MoveIT
```bash
# TURN OFF YOUR WIFI
ros2 launch ur_moveit_config ur_moveit.launch.py ur_type:=ur3e launch_rviz:=true launch_servo:=true
ros2 control switch_controllers --activate forward_position_controller --deactivate scaled_joint_trajectory_controller --deactivate  joint_trajectory_controller # IF USING SERVO (This package uses servo)
ros2 service call /servo_node/start_servo std_srvs/srv/Trigger {} # Start the servo process

ros2 service call /servo_node/stop_servo std_srvs/srv/Trigger {} #Stop when done

####### Alternative ########
ros2 control switch_controllers --activate scaled_joint_trajectory_controller --deactivate joint_trajectory_controller # IF USING GOAL PLANNING
```
### On Teach Pendant
1. Installation > URCaps > External Control
    - Enter your ip into HostIP & Host name
```bash
# Second entry reported (usually 192.168.0.xxx)
hostname -I
```
2. Program > URCaps > External Control
    - Make sure "Control by [YOUR_IP]" is the program selected

3. In the bottom left, start the robot

4. Installation > URCaps > OnRobot Setup
    - Make sure device info is set to "No Connection"

5. Click the start button in the bottom right, and start the RobotProgram

## Extracting Robot Calibration
```bash
# Replace the IP address with the IP address of your actual robot / URSim
# Calibration extraction will be saved to the target_filename
ros2 launch ur_calibration calibration_correction.launch.py robot_ip:=<robot_ip> target_filename:="${HOME}/my_robot_calibration.yaml"
```