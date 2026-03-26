from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    ld = LaunchDescription()

    sample_node = Node(
            package='jamc',
            executable='controller',
            output='screen',
        )
    ld.add_action(sample_node)

    return ld

