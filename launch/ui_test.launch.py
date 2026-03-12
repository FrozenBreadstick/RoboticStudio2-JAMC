from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    ld = LaunchDescription()

    ui_node = Node(
            package='jamc',
            executable='ui',
            output='screen'
        )
    ld.add_action(ui_node)

    return ld