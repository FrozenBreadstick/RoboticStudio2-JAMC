#include "sample.h"

namespace Sample
{
    SampleClass::SampleClass() : Node("sample_node")
    {
        RCLCPP_INFO(this->get_logger(), "Sample Node Started");
        _sample_service = this->create_service<jamc::srv::SampleService>
        ("sample_service", std::bind(&SampleClass::sample_service_handler, this, std::placeholders::_1, std::placeholders::_2));

        for(int i = 0; i < 10; i++)
        {
            numbers_[i] = i + 10;
        }
    }

    SampleClass::~SampleClass()
    {
        RCLCPP_INFO(this->get_logger(), "Sample Node Stopped");
    }

    void SampleClass::sample_service_handler(const std::shared_ptr<jamc::srv::SampleService::Request> request, 
                                               std::shared_ptr<jamc::srv::SampleService::Response> response)
    {
        RCLCPP_INFO(this->get_logger(), "Sample Service Called with request: %d", request->index);
        if(request->index >= 0 && request->index < 10)
        {
            RCLCPP_INFO(this->get_logger(), "Sample Service Responding with: %s", response->message.c_str());
            response->message = "The number is: " + std::to_string(numbers_[request->index]);
        }
        else
        {
            RCLCPP_WARN(this->get_logger(), "Sample Service Received Invalid Index: %d", request->index);
            response->message = "Index out of bounds. Please provide an index between 0 and 9.";
        }
    }
}