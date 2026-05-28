from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, RegisterEventHandler, ExecuteProcess, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.event_handlers import OnShutdown

def create_ur_driver(): # Launch the UR_Driver
    return IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([
                FindPackageShare("ur_robot_driver"),
                "launch",
                "ur_control.launch.py"
            ])
        ),
        launch_arguments={
            "ur_type": LaunchConfiguration("ur_type"),
            "robot_ip": LaunchConfiguration("robot_ip"),
            "launch_rviz": "false",
        }.items()
    )

def start_moveit():  # Launch MoveIt

    moveit_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([
                FindPackageShare("ur_moveit_config"),
                "launch",
                "ur_moveit.launch.py"
            ])
        ),
        launch_arguments={
            "ur_type": LaunchConfiguration("ur_type"),
            "launch_rviz": "true",
            "launch_servo": "true",
        }.items()
    )

    #Delay startup until UR driver is initialized
    return TimerAction(
        period=10.0,
        actions=[moveit_launch]
    )

def switch_to_servo_controller():

    switch_cmd = ExecuteProcess(
        cmd=[
            'ros2',
            'control',
            'switch_controllers',
            '--activate',
            'forward_position_controller',
            '--deactivate',
            'scaled_joint_trajectory_controller',
            '--deactivate',
            'joint_trajectory_controller'
        ],
        output='screen'
    )

    # Delay to ensure controllers are available
    return TimerAction(
        period=40.0,
        actions=[switch_cmd]
    )

def start_servo():

    servo_cmd = ExecuteProcess(
        cmd=[
            'ros2',
            'service',
            'call',
            '/servo_node/start_servo',
            'std_srvs/srv/Trigger',
            '{}'
        ],
        output='screen'
    )

    return TimerAction(
        period=45.0,
        actions=[servo_cmd]
    )

def make_ui():
    ui = Node(
            package='jamc',
            executable='ui',
            output='screen',
        )
    
    return ui

# TODO
# def make_perception():
    # perception = Node(
    #     package='jamc',
    #     executable='perception',
    #     output='screen',
    # )

    # return TimerAction(
    #     period=19.0,
    #     actions=[perception]
    # )

def make_controller():
    controller = Node(
            package='jamc',
            executable='controller',
            output='screen',
        )
    
    return TimerAction(
        period=50.0,
        actions=[controller]
    )

def shutdown_actions():

    stop_servo = ExecuteProcess(
        cmd=[
            'ros2',
            'service',
            'call',
            '/servo_node/stop_servo',
            'std_srvs/srv/Trigger',
            '{}'
        ],
        output='screen'
    )

    return RegisterEventHandler(
        OnShutdown(
            on_shutdown=[
                stop_servo
            ]
        )
    )

def generate_launch_description():

    ld = LaunchDescription()

    ld.add_action(
        DeclareLaunchArgument(
            "robot_ip",
            default_value="192.168.0.194"
        )
    )

    ld.add_action(
        DeclareLaunchArgument(
            "ur_type",
            default_value="ur3e"
        )
    )

    ld.add_action(create_ur_driver())

    ld.add_action(start_moveit())

    ld.add_action(switch_to_servo_controller())

    ld.add_action(start_servo())

    # ld.add_action(make_perception())

    ld.add_action(make_ui())

    ld.add_action(make_controller())

    ld.add_action(shutdown_actions())

    return ld

