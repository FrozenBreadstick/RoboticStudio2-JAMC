UR_README.md
# How to use the Controller Class
<hr style="border: none; border-top: 3px double #558bff;">

## 1. Installation

#### UR Driver Installtion
```bash
sudo apt-get install ros-humble-ur
```
---
#### Motion Planner Installation
```bash
sudo apt install ros-humble-moveit
sudo apt install ros-humble-ros2-control ros-humble-ros2-controllers
```

<hr style="border: none; border-top: 3px double #558bff;">

## 2. Physical Robot Setup

#### On Teach Pendant
1. Installation > URCaps > External Control
    - Enter your ip into HostIP & Host name
```bash
# Second entry reported (usually 192.168.0.xxx)
hostname -I
```
2. Program > URCaps > External Control
    - Make sure "Control by [YOUR_IP]" is the program selected

3. In the bottom left, start the robot (Ensure payload is set to 150 grams)

4. Installation > URCaps > OnRobot Setup
    - Make sure device info is set to "No Connection"

5. Click the start button in the bottom right, and start the RobotProgram

<hr style="border: none; border-top: 3px double #558bff;">

## 3. Software Setup

### Automatic
#### Running the Built in Launch File
```bash
# Once the repository is compiled as per build instructions
ros2 launch jamc all.launch.py
```

---

### Manual
#### Running the Driver
```bash
# Replace the IP address with the IP address of your actual robot / URSim
ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3e robot_ip:=192.168.0.191 launch_rviz:=false
```
---
#### Running MoveIT
```bash
# TURN OFF YOUR WIFI
ros2 launch ur_moveit_config ur_moveit.launch.py ur_type:=ur3e launch_rviz:=true launch_servo:=true
ros2 control switch_controllers --activate forward_position_controller --deactivate scaled_joint_trajectory_controller --deactivate  joint_trajectory_controller # IF USING SERVO (This package uses servo)
ros2 service call /servo_node/start_servo std_srvs/srv/Trigger {} # Start the servo process

ros2 service call /servo_node/stop_servo std_srvs/srv/Trigger {} #Stop when done

####### Alternative ########
ros2 control switch_controllers --activate scaled_joint_trajectory_controller --deactivate joint_trajectory_controller # IF USING GOAL PLANNING
```
---
## Extracting Robot Calibration
```bash
# Replace the IP address with the IP address of your actual robot / URSim
# Calibration extraction will be saved to the target_filename
ros2 launch ur_calibration calibration_correction.launch.py robot_ip:=<robot_ip> target_filename:="${HOME}/my_robot_calibration.yaml"
```

<hr style="border: none; border-top: 3px double #558bff;">

## 4. Using the Controller Class in Code

#### How to start the node
```C++
rclcpp::init(argc, argv); //Initialise ROS2
auto controller_node = std::make_shared<Control::Controller>(); //Create an instance of the controller class node
rclcpp::spin(controller_node); //Spin the node into ROS2
```
The Controller class exposes no public class members and as such can only be instantiated and destroyed.
See below for a list of exposed ROS2 interfaces that allow interaction when the node is running.

#### Interface List
| Interface Name | Topic/Service Name | Type | Description |
| :--- | :--- | :--- | :--- |
| **Services** | | | |
| `load_service_` | `/MIPI/load` | `jamc::srv::Load` | Uses the custom "Load" service, takes a filepath and an index corresponding to the instrument channel to load |
| `time_service_` | `/MIPI/time_scale` | `jamc::srv::TimeScale` | Uses the custom TimeScale service, takes a float between 0 and 1 that corresponds to the playback speed the robot should be playing at |
| `play_pause_service_` | `/MIPI/play_pause` | `jamc::srv::Func` | Uses the custom Func service, takes an empty that causes the node to toggle playback of the robot |
| `play_direction_service_` | `/MIPI/direction` | `jamc::srv::Func` | Uses the custom Func service, takes an empty that causes the node to toggle playback direction of the robot |
| **Subscribers** | | | |
| `debug_target_sub_` | `/debug_target` | `geometry_msgs::msg::Point` | Anytime a point message is received here it will trigger the robot to attempt to move the end effector, via servo, by treating that point as a direction vector (utilising only the x and y) |
| `key_positions_sub_` | `/piano_keys` | `geometry_msgs::msg::PoseArray` | Expects a PoseArray msg of size 37. Updates an internal variable used to find the position of the keys in relation to the camera. Referred to in the control loop when calculating the X and Y velocity |
| `joint_state_sub_` | `/joint_states` | `sensor_msgs::msg::JointState` | Expects a JointState msg. This is a topic that is normally created when the UR3e Driver is started and reflects the current joint poisitions of the robot |
| **Publishers** | | | |
| `twist_pub_` | `/servo_node/delta_twist_cmds` | `geometry_msgs::msg::TwistStamped` | Publishes to the MoveIt servo node for streaming End Effector velocities |
| `joint_traj_streaming_pub_` | `/servo_server/delta_joint_cmds` | `control_msgs::msg::JointJog` | Publishes to the MoveIt servo_server to directly stream joint velocities. Used in the startup sequence to bring the robot into position |