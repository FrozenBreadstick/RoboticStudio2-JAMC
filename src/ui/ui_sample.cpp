#include "ui/ui_sample.h"

namespace UI
{
    SampleUI::SampleUI() : QWidget(), Node("sample_ui") {

        RCLCPP_INFO(this->get_logger(), "Sample UI Started");
        _sample_client = this->create_client<jamc::srv::SampleService>("sample_service");

        auto* layout = new QVBoxLayout(this);
        _label = new QLabel("", this);
        _input = new QSpinBox(this);
        _button = new QPushButton("Send Number", this);

        layout->addWidget(_label);
        layout->addWidget(_input);
        layout->addWidget(_button);

        connect(_button, &QPushButton::clicked, this, &SampleUI::send_request);
    }

    SampleUI::~SampleUI() {
        RCLCPP_INFO(this->get_logger(), "Sample UI Stopped");
    }

    void SampleUI::send_request() {
        auto request = std::make_shared<jamc::srv::SampleService::Request>();
        request->index = _input->value();
        RCLCPP_INFO(this->get_logger(), "Sending request with index: %d", request->index);

        //Check once without blocking the UI indefinitely
        if (!_sample_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Service is not available.");
            _label->setText("Error: Service unavailable"); //Update UI to reflect state
            return;
        }

        _sample_client->async_send_request(request, 
                [this](rclcpp::Client<jamc::srv::SampleService>::SharedFuture future) {
                    auto response = future.get();
                    QMetaObject::invokeMethod(this->_label, "setText", 
                        Q_ARG(QString, QString::fromStdString(response->message)));
                });
    }
}