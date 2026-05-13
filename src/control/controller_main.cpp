#include <rclcpp/rclcpp.hpp>
#include "control/controller.h"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto controller_node = std::make_shared<Control::Controller>();
    rclcpp::spin(controller_node);
    return 0;
}