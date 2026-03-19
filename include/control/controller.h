#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "external/json.hpp"
#include "rclcpp/rclcpp.hpp"
#include "srv/load.hpp"
#include "srv/func.hpp"
#include "srv/time_scale.hpp"

using nlohmann::json;

namespace Control
{
    class Controller : public rclcpp::Node
    {
    public:
        Controller();
        ~Controller();

    private:
        

    };
}


#endif // CONTROLLER_H