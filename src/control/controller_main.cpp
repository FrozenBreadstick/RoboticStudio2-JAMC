// #include <rclcpp/rclcpp.hpp>

// int main(int argc, char* argv[])
// {
//     auto argc_ = argc; // DO NOT WORRY ABOUT THESE WARNINGS REGARDING UNUSED VARIABLES
//     auto argv_ = argv; // DO NOT WORRY ABOUT THESE WARNINGS REGARDING UNUSED VARIABLES
//     return 0;
// }

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

class ServoLerpTest : public rclcpp::Node
{
public:
    ServoLerpTest() : Node("servo_lerp_test")
    {
        twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(
            "/servo_node/delta_twist_cmds", 10);

        timer_ = this->create_wall_timer(
            10ms, std::bind(&ServoLerpTest::publishVelocity, this));

        start_time_ = this->now();
        RCLCPP_INFO(this->get_logger(), "Starting open-loop lerp test via Servo");
    }

private:
    void publishVelocity()
    {
        auto msg = geometry_msgs::msg::TwistStamped();
        msg.header.stamp = this->now();
        msg.header.frame_id = "base_link"; 

        auto current_time = this->now();
        double duration = (current_time - start_time_).seconds();

        // --- Circle Parameters ---
        double radius = 1; // 10 cm radius
        double omega = 1;  // Radians per second (speed of the circle)

        if (duration < 30.0)
        {
            msg.twist.linear.x = -radius * omega * std::sin(omega * duration);
            msg.twist.linear.z = radius * omega * std::cos(omega * duration);
            msg.twist.linear.y = 0.0; // Keep it flat in the XY plane

            // Keep orientation locked (no rotation, great for cameras!)
            msg.twist.angular.x = 0.0;
            msg.twist.angular.y = 0.0;
            msg.twist.angular.z = 0.0;
        }
        else
        {
            // Halt
            msg.twist.linear.x = 0.0;
            msg.twist.linear.y = 0.0;
            msg.twist.angular.x = 0.0;
            msg.twist.angular.y = 0.0;
            msg.twist.angular.z = 0.0;
            
            if (!stopped_) {
                RCLCPP_INFO(this->get_logger(), "Circle complete. Halting.");
                stopped_ = true;
            }
        }

        twist_pub_->publish(msg);
    }

    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Time start_time_;
    bool stopped_ = false;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ServoLerpTest>());
    rclcpp::shutdown();
    return 0;
}