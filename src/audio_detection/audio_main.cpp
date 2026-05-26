#include <rclcpp/rclcpp.hpp>
#include "audio/audio.h"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto audio_node = std::make_shared<Audio::Audio>();
    rclcpp::spin(audio_node);
    return 0;
}