#include <rclcpp/rclcpp.hpp>
#include "sample.h"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);

    auto sample_node = std::make_shared<Sample::SampleClass>();

    rclcpp::spin(sample_node);

    return 0;
}