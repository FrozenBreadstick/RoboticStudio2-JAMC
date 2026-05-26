#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseArray, Pose
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from rclpy.executors import MultiThreadedExecutor

from end_effector_streamer import EndEffectorStreamer

"""!
@brief The UR3eOffsetPublisher node, sets up TF2 listener, and simulates a piano keyboard and the perception system by publishing preset target poses and their offsets relative to the end-effector position.
"""
class UR3eOffsetPublisher(Node):
    """!
    @brief Initializes the UR3eOffsetPublisher node 
    """
    def __init__(self):
        super().__init__('ur3e_offset_publisher')

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        self.offset_pub = self.create_publisher(PoseArray, '/piano_keys', 10)
        self.key_pos = self.create_publisher(PoseArray, '/key_positions', 10)

        self.preset_poses = self.generate_preset_poses()

        self.timer = self.create_timer(0.1, self.timer_callback)
        
        self.get_logger().info("UR3e Offset Publisher Node has started.")

    """!
    @brief Generates a PoseArray of preset target poses representing piano keys, with X/Y offsets to simulate the perception system's output. The keys are arranged in a standard piano layout with appropriate spacing for black and white keys.
    """
    def generate_preset_poses(self):
        pose_array = PoseArray()
        pose_array.header.frame_id = 'base_link'
        pose_array.header.stamp = self.get_clock().now().to_msg()
        
        # 1 3 5 8 10 13 15 17 20 22 25 27 29 32 34
        black_keys = [1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34]
        x_offset = 0

        for i in range(37):
            pose = Pose()
            x_offset += 0.01 if i or i-1 in black_keys else 0.02
            y_offset = 0.0 if i in black_keys else -0.02
            pose.position.x = -0.2 + (x_offset)
            pose.position.y = 0.45 + (y_offset)
            pose.position.z = 0.0
            pose.orientation.w = 1.0
            
            pose_array.poses.append(pose)
            
        return pose_array

    """!
    @brief Timer callback that calculates the offset from the end effector to each preset key pose and publishes it as a PoseArray to /piano_keys. It also publishes the absolute positions of the keys to /key_positions for visualization.
    """
    def timer_callback(self):
        target_frame = 'tool0' #End-effector frame
        source_frame = 'base_link' #Robot base frame

        self.preset_poses.header.stamp = self.get_clock().now().to_msg()
        self.key_pos.publish(self.preset_poses)

        try:
            now = rclpy.time.Time()
            transform = self.tf_buffer.lookup_transform(
                source_frame,
                target_frame,
                now
            )
            
            ee_x = transform.transform.translation.x
            ee_y = transform.transform.translation.y
            
        except TransformException as ex:
            self.get_logger().warning(f'Could not transform {source_frame} to {target_frame}: {ex}')
            return

        offset_pose_array = PoseArray()
        offset_pose_array.header.stamp = self.get_clock().now().to_msg()
        offset_pose_array.header.frame_id = 'base_link'
        
        # Calculate X/Y offsets relative to the current end effector position
        for target_pose in self.preset_poses.poses:
            offset_pose = Pose()
            
            # The X distance from the end effector
            dx = target_pose.position.x - ee_x
            
            # Apply Field of View limitations for the camera simulation
            if dx < -0.1:
                # Key is out of view to the left
                offset_pose.position.x = float('-inf')
                offset_pose.position.y = float('-inf')
            elif dx > 0.1:
                # Key is out of view to the right
                offset_pose.position.x = float('inf')
                offset_pose.position.y = float('inf')
            else:
                # Key is visible! Calculate normal offset
                offset_pose.position.x = dx
                offset_pose.position.y = target_pose.position.y - ee_y
            
            offset_pose.position.z = 0.0
            
            # Maintaining basic orientation identity for the offset map
            offset_pose.orientation.w = 1.0
            
            offset_pose_array.poses.append(offset_pose)

        self.offset_pub.publish(offset_pose_array)

def main(args=None):
    rclpy.init(args=args)
    node = UR3eOffsetPublisher()
    end_effector_streamer = EndEffectorStreamer()
    executor = MultiThreadedExecutor()
    executor.add_node(node)
    executor.add_node(end_effector_streamer)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()