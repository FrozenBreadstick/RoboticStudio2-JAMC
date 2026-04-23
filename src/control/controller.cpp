#include "control/controller.h"

/**
 * @brief Standalone Class for Controlling a UR3e. Typically reads data from .mipi files for plaback on a piano. Exposes various services for control.
 * 
 * @details This class contains the following ROS2 I/O
 * - **Services**
 *  - '-/MIPI/load' (jamc/srv/Load): Takes a string which should be the filepath to a .mipi file to be loaded, and an integer that is the index of the instrument to play
 *  - '-/MIPI/time_scale' (jamc/srv/TimeScale): Take a float between 0 and 1 used to scale the speed of playback
 *  - '-/MIPI/play_pause' (jamc/srv/Func): Takes an empty, this is a trigger service used to play and pause the playback
 *  - '-/MIPI/direction' (jamc/srv/Func): Takes an empty, this is a trigger service used to toggle the direction of playback
 */
Control::Controller::Controller() : Node("MIPI_Controller"), connor()
{
    RCLCPP_INFO(this->get_logger(), "Controller node has been started.");
    twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/servo_node/delta_twist_cmds", 10);
    debug_target_sub_ = this->create_subscription<geometry_msgs::msg::Point>("/debug_target", 10, std::bind(&Controller::debug_target_callback, this, std::placeholders::_1));

    load_service_ = this->create_service<jamc::srv::Load>("/MIPI/load", std::bind(&Controller::load_callback, this, std::placeholders::_1, std::placeholders::_2));
    time_service_ = this->create_service<jamc::srv::TimeScale>("/MIPI/time_scale", std::bind(&Controller::time_scale_callback, this, std::placeholders::_1, std::placeholders::_2));
    play_pause_service_ = this->create_service<jamc::srv::Func>("/MIPI/play_pause", std::bind(&Controller::play_pause_callback, this, std::placeholders::_1, std::placeholders::_2));
    play_direction_service_ = this->create_service<jamc::srv::Func>("/MIPI/direction", std::bind(&Controller::play_direction_callback, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief Standard Destructor for Controller Class
 */
Control::Controller::~Controller()
{
    RCLCPP_INFO(this->get_logger(), "Controller node is shutting down.");
}

/**
 * @brief A convenience method that neatly packs up a twist messge and publishes it to the UR_Driver
 * @param x A double for translational velocity of the End Effector in the **X** axis
 * @param y A double for translational velocity of the End Effector in the **Y** axis
 * @param z A double for translational velocity of the End Effector in the **Z** axis
 * @param angular_x A double for rotational velocity of the End Effector in the **X** axis (Optional, Defaults to 0.0)
 * @param angular_y A double for rotational velocity of the End Effector in the **Y** axis (Optional, Defaults to 0.0)
 * @param angular_z A double for rotational velocity of the End Effector in the **Z** axis (Optional, Defaults to 0.0)
 */
void Control::Controller::sendTwistMsg(double x, double y, double z, double angular_x, double angular_y, double angular_z)
{
    auto msg = geometry_msgs::msg::TwistStamped();
    msg.header.stamp = this->now();
    msg.header.frame_id = "base_link"; 

    msg.twist.linear.x = x;
    msg.twist.linear.y = y;
    msg.twist.linear.z = z;

    msg.twist.angular.x = angular_x;
    msg.twist.angular.y = angular_y;
    msg.twist.angular.z = angular_z;

    twist_pub_->publish(msg);

    RCLCPP_INFO(this->get_logger(), "Sending Twist message: linear=(%.2f, %.2f, %.2f), angular=(%.2f, %.2f, %.2f)", x, y, z, angular_x, angular_y, angular_z);
}

/**
 * @brief The public method for sending translational End Effector vector commands to the UR3e
 * @param vec Takes a vector3 struct (included in this class, or declared using {x, y, z})
 */
void Control::Controller::sendVector(const vector3 &vec)
{
    sendTwistMsg(vec.x, vec.y, vec.z);
}

/**
 * @brief A simple helper method for immediately sending stop to the UR3e
 */
void Control::Controller::sendStop()
{
    sendTwistMsg(0.0, 0.0, 0.0);
}

/**
 * @brief A method for enabling "activeTrack" mode, allowing for live visual servoing following a topic
 * @param target A vector3 struct that is the target
 * @param x A bool to enable motion in the X plane
 * @param y A bool to enable motion in the Y plane
 * @param z A bool to enable motion in the Z plane
 */
int Control::Controller::activeTrackDebug(vector3 target, bool x, bool y, bool z)
{
    if (!x) target.x = 0.0;
    if (!y) target.y = 0.0;
    if (!z) target.z = 0.0;
    sendVector(target);
    return 0;
}

/**
 * @brief Callback function for catching the debug subscriber, used to activate activeTrack
 * @param msg Input from the topic subscription
 */
void Control::Controller::debug_target_callback(const geometry_msgs::msg::Point::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "Received debug target: (%.2f, %.2f, %.2f)", msg->x, msg->y, msg->z);

    double scaling = 4;
    vector3 vec;

    vec.x = msg->x;
    vec.y = msg->y;
    vec.z = msg->z;

    // Clamp to safe range
    double max_val = std::max({std::abs(vec.x), std::abs(vec.y), std::abs(vec.z)});
    if (max_val > 1.0) {
        vec.x /= max_val;
        vec.y /= max_val;
        vec.z /= max_val;
    }

    // Apply scaling (velocity gain)
    vec.x *= scaling;
    vec.y *= scaling;
    vec.z *= scaling;

    double deadzone = 0.05; //prevent jitter
    if (std::abs(vec.x) < deadzone) vec.x = 0.0;
    if (std::abs(vec.y) < deadzone) vec.y = 0.0;

    activeTrackDebug(vec, true, true, false);
}

/**
 * @brief Callback function for the Load Service, used to load .mipi files and select which instrument channel
 * @param request The request from the service call, containing the filepath and instrument index
 * @param response The response for the service call, containing a message
 */
void Control::Controller::load_callback(const std::shared_ptr<jamc::srv::Load::Request> request, std::shared_ptr<jamc::srv::Load::Response> response)
{
    RCLCPP_INFO(this->get_logger(), "Received load request: filepath=%s, instrument_index=%d", request->filepath.c_str(), request->index);
    song_ = connor.get_channel_notes()[request->index];
    note_timings_ = connor.get_channel_note_timings()[request->index];
    note_durations_ = connor.get_channel_note_durations()[request->index];
    response->message = "[SUCCESS] Loaded file: " + request->filepath + " with instrument index: " + std::to_string(request->index);
}

/**
 * @brief Callback function for the TimeScale Service, used to set the time scaling factor for playback
 * @param request The request from the service call, containing the time scale factor
 * @param response The response for the service call, containing a message
 */
void Control::Controller::time_scale_callback(const std::shared_ptr<jamc::srv::TimeScale::Request> request, std::shared_ptr<jamc::srv::TimeScale::Response> response)
{
    RCLCPP_INFO(this->get_logger(), "Received time scale request: time_scale=%.2f", request->scale);
    time_scale_ = request->scale;
    response->message = "[SUCCESS] Set time scale to: " + std::to_string(request->scale);
}

/**
 * @brief Callback function for the Play/Pause Service, used to toggle playback of the current track
 * @param request The request from the service call, empty for this trigger service
 * @param response The response for the service call, containing a message
 */
void Control::Controller::play_pause_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response)
{
    (void)request; // Unused parameter
    play_ = !play_;
    RCLCPP_INFO(this->get_logger(), "Toggled play/pause. Now playing: %s", play_ ? "true" : "false");
    response->message = std::string("[SUCCESS] Toggled play/pause. Now playing: ") + (play_ ? "true" : "false");
}

/**
 * @brief Callback function for the Play Direction Service, used to toggle the direction of playback
 * @param request The request from the service call, empty for this trigger service
 * @param response The response for the service call, containing a message
 */
void Control::Controller::play_direction_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response)
{    (void)request; // Unused parameter
    direction_ = !direction_;
    RCLCPP_INFO(this->get_logger(), "Toggled play direction. Now playing: %s", direction_ ? "forward" : "backward");
    response->message = std::string("[SUCCESS] Toggled play direction. Now playing: ") + (direction_ ? "forward" : "backward");
}