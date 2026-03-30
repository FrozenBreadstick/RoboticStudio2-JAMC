#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "external/json.hpp"
#include "rclcpp/rclcpp.hpp"
#include "jamc/srv/load.hpp"
#include "jamc/srv/func.hpp"
#include "jamc/srv/time_scale.hpp"
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <chrono>
#include <cmath>

using nlohmann::json;

namespace Control
{
    struct vector3
    {
        double x;
        double y;
        double z;
    };

    class Controller : public rclcpp::Node
    {
    public:
        Controller();
        ~Controller();

        void sendVector(const vector3 &vec);
        void sendStop();

        int activeTrackDebug(vector3 target, bool x = false, bool y = false, bool z = false);

    private:
        //Methods
        void sendTwistMsg(double x, double y, double z, double angular_x = 0.0, double angular_y = 0.0, double angular_z = 0.0);

        //Publishers
        rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;

        //Subscribers
        rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr debug_target_sub_;
        void debug_target_callback(const geometry_msgs::msg::Point::SharedPtr msg);

        //Services
        /**
         * @name ROS2 Services
         * Services that the Controller Class provides
         */
        ///@{
        rclcpp::Service<jamc::srv::Load>::SharedPtr load_service_;
        rclcpp::Service<jamc::srv::TimeScale>::SharedPtr time_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr play_pause_service_;
        ///@}


        //Timers
        rclcpp::TimerBase::SharedPtr control_timer_;

    };
}


#endif // CONTROLLER_H