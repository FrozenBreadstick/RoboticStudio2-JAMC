#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Point
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

"""!
@brief A ROS2 Python Node that interfaces with TF2 to stream the End Effector position so that we don't have to do it in C++
- Runs by itself if the perception node is running.
- Is instantiated by controller_tester.py if running with simulated perception.
"""
class EndEffectorStreamer(Node):
    """!
    @brief Initialises the EndEffectorStreamer node, sets up TF2 listener, and creates a publisher for the end-effector position.
    """
    def __init__(self):
        super().__init__('end_effector_streamer')

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        self.ee_pub = self.create_publisher(Point, '/MIPI/EE', 10)

        self.timer = self.create_timer(0.01, self.timer_callback)
        
        self.get_logger().info("End Effector Streamer Node has started.")

    """!
    @brief Timer callback that looks up the transform from 'base_link' to 'tool0', extracts the end-effector position, and publishes it as a Point to /MIPI/EE
    """
    def timer_callback(self):
        source_frame = 'base_link'
        target_frame = 'tool0'
        try:
            now = rclpy.time.Time()
            transform = self.tf_buffer.lookup_transform(
                source_frame,
                target_frame,
                now
            )
        except TransformException as ex:
            self.get_logger().warning(f'Could not transform {source_frame} to {target_frame}: {ex}')
            return

        ee_pose = Point()

        ee_pose.x = transform.transform.translation.x
        ee_pose.y = transform.transform.translation.y
        ee_pose.z = transform.transform.translation.z

        self.ee_pub.publish(ee_pose)

def main(args=None):
    rclpy.init(args=args)
    node = EndEffectorStreamer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()