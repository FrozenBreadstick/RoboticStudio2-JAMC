#include <rclcpp/rclcpp.hpp>
#include <QApplication>
#include <thread>
#include "ui/ui_sample.h"

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv); //Init ROS2 first
    
    QApplication app(argc, argv); //Init Qt application

    auto ui_node = std::make_shared<UI::SampleUI>(); //Make the UI by instantiating the node
    ui_node->show();

    std::thread ros_thread([ui_node]() {
        rclcpp::spin(ui_node);
    }); //Set ros processes to spin in a seperate thread to avoid blocking the UI

    int exec_code = app.exec(); //Start the Qt event loop

    rclcpp::shutdown(); //SHutdown once app is closed
    ros_thread.join();

    return exec_code;
}