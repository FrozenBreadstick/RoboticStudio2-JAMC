#ifndef UI_SAMPLE_H
#define UI_SAMPLE_H

#include "rclcpp/rclcpp.hpp"
#include "jamc/srv/sample_service.hpp"
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace UI
{
    class SampleUI : public QWidget, public rclcpp::Node {
        Q_OBJECT
        public:
            SampleUI();
            ~SampleUI();

        private:
            void send_request();

            rclcpp::Client<jamc::srv::SampleService>::SharedPtr _sample_client;
            QLabel* _label;
            QSpinBox* _input;
            QPushButton* _button;
    };
}

#endif // UI_SAMPLE_H