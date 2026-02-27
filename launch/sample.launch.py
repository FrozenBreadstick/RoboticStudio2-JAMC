from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    ld = LaunchDescription()

    sample_node = Node(
            package='jamc',
            executable='sample',
            output='screen',
        )
    ld.add_action(sample_node)

    ui_sample_node = Node(
            package='jamc',
            executable='ui',
            output='screen'
        )
    ld.add_action(ui_sample_node)

    return ld

