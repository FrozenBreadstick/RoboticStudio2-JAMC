#ifndef SAMPLE_H
#define SAMPLE_H

#include "rclcpp/rclcpp.hpp"
#include "jamc/srv/sample_service.hpp"

namespace Sample
{
    class SampleClass : public rclcpp::Node
    {
        public:
            SampleClass();
            ~SampleClass();

        private:
            rclcpp::Service<jamc::srv::SampleService>::SharedPtr _sample_service;
            void sample_service_handler(const std::shared_ptr<jamc::srv::SampleService::Request> request, 
                                              std::shared_ptr<jamc::srv::SampleService::Response> response);

            int numbers_[10];
    };
}

#endif // SAMPLE_H