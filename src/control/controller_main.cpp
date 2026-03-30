#include <rclcpp/rclcpp.hpp>
#include "control/controller.h"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto controller_node = std::make_shared<Control::Controller>();
    rclcpp::spin(controller_node);
    return 0;
}

// #include <rclcpp/rclcpp.hpp>
// #include <geometry_msgs/msg/twist_stamped.hpp>
// #include <chrono>
// #include <cmath>

// using namespace std::chrono_literals;

// class ServoLerpTest : public rclcpp::Node
// {
// public:
//     ServoLerpTest() : Node("servo_lerp_test")
//     {
//         twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(
//             "/servo_node/delta_twist_cmds", 10);

//         timer_ = this->create_wall_timer(
//             10ms, std::bind(&ServoLerpTest::publishVelocity, this));

//         start_time_ = this->now();
//         RCLCPP_INFO(this->get_logger(), "Starting open-loop lerp test via Servo");
//     }

// private:
//     void publishVelocity()
//     {
//         auto msg = geometry_msgs::msg::TwistStamped();
//         msg.header.stamp = this->now();
//         msg.header.frame_id = "base_link"; 

//         auto current_time = this->now();
//         double duration = (current_time - start_time_).seconds();

//         // --- Circle Parameters ---
//         double radius = 1; // 10 cm radius
//         double omega = 1;  // Radians per second (speed of the circle)

//         if (duration < 30.0)
//         {
//             msg.twist.linear.x = -radius * omega * std::sin(omega * duration);
//             msg.twist.linear.z = radius * omega * std::cos(omega * duration);
//             msg.twist.linear.y = 0.0; // Keep it flat in the XY plane

//             // Keep orientation locked (no rotation, great for cameras!)
//             msg.twist.angular.x = 0.0;
//             msg.twist.angular.y = 0.0;
//             msg.twist.angular.z = 0.0;
//         }
//         else
//         {
//             // Halt
//             msg.twist.linear.x = 0.0;
//             msg.twist.linear.y = 0.0;
//             msg.twist.angular.x = 0.0;
//             msg.twist.angular.y = 0.0;
//             msg.twist.angular.z = 0.0;
            
//             if (!stopped_) {
//                 RCLCPP_INFO(this->get_logger(), "Circle complete. Halting.");
//                 stopped_ = true;
//             }
//         }

//         twist_pub_->publish(msg);
//     }

//     rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;
//     rclcpp::TimerBase::SharedPtr timer_;
//     rclcpp::Time start_time_;
//     bool stopped_ = false;
// };

// int main(int argc, char **argv)
// {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<ServoLerpTest>());
//     rclcpp::shutdown();
//     return 0;
// }

/** TODO
 * 1. Create class from above code
 * 2. Create method that publishes velocity vectors determined by comparing current EE position to target position
 * 3. Create service that accepts target position and updates the target position used by the method in #2
 * 4. Create visual servoing test class
 * 4. Create service that accepts time scale factor and updates the speed of the published velocity vectors accordingly
 * 5. Create service that accepts a JSON string containing a list of target positions and pause/hold times
 *  */