#include "control/controller.h"
using Control::vector3;
using Control::STATE;

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
Control::Controller::Controller() : Node("MIPI_Controller"), CONTROL_TIME(0), LAST_CONTROL_TIME_POINT(Clock::now()), state_(STATE::WAITING), connor()
{
    RCLCPP_INFO(this->get_logger(), "Controller node has been started.");
    twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/servo_node/delta_twist_cmds", 10);
    debug_target_sub_ = this->create_subscription<geometry_msgs::msg::Point>("/debug_target", 10, std::bind(&Controller::debug_target_callback, this, std::placeholders::_1));
    key_positions_sub_ = this->create_subscription<geometry_msgs::msg::PoseArray>("/piano_keys", 10, std::bind(&Controller::key_positions_callback, this, std::placeholders::_1));

    load_service_ = this->create_service<jamc::srv::Load>("/MIPI/load", std::bind(&Controller::load_callback, this, std::placeholders::_1, std::placeholders::_2));
    time_service_ = this->create_service<jamc::srv::TimeScale>("/MIPI/time_scale", std::bind(&Controller::time_scale_callback, this, std::placeholders::_1, std::placeholders::_2));
    play_pause_service_ = this->create_service<jamc::srv::Func>("/MIPI/play_pause", std::bind(&Controller::play_pause_callback, this, std::placeholders::_1, std::placeholders::_2));
    play_direction_service_ = this->create_service<jamc::srv::Func>("/MIPI/direction", std::bind(&Controller::play_direction_callback, this, std::placeholders::_1, std::placeholders::_2));

    control_timer_ = this->create_wall_timer(std::chrono::milliseconds(25), std::bind(&Controller::control_loop, this));
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

    {
        std::lock_guard<std::mutex> lock(song_mutex_);
        if(!play_) {
            return; //If not playing or no song loaded, skip the control loop
        }
    }

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
 * @brief Velocity calculation for control loop
 * @param note The MIDI note value for the current note being played
 */
std::optional<vector3> Control::Controller::calculate_velocity(int note)
{
    double scaling = 1; //Start small, controlled via parameter later
    double z = calculate_z();
    vector3 target;
    {
        std::lock_guard<std::mutex> lock(key_positions_mutex_);
        if (key_positions_.poses.empty()) {
            RCLCPP_WARN(this->get_logger(), "Key positions are empty, cannot calculate velocity.");
            return vector3{0.0, 0.0, 0.0};
        };
        if (note < 0 || note >= static_cast<int>(key_positions_.poses.size())) {
            RCLCPP_WARN(this->get_logger(), "Note index %d is out of bounds for key positions. (Somehow?)", note);
            return vector3{0.0, 0.0, 0.0};
        }
        auto target_pose = key_positions_.poses[note];
        target.x = target_pose.position.x;
        target.y = target_pose.position.y;
        target.z = z;
    }

    double max_val = std::max({std::abs(target.x), std::abs(target.y), std::abs(target.z)});
    if (max_val > 1.0) {
        target.x /= max_val;
        target.y /= max_val;
        target.z /= max_val;
    }

    // Apply scaling (velocity gain)
    target.x *= scaling;
    target.y *= scaling;
    target.z *= scaling;

    double deadzone = 0.05; //prevent jitter & abort when goal reached
    if (std::abs(target.x) < deadzone)  target.x = 0.0;
    if (std::abs(target.y) < deadzone)  target.y = 0.0;
    if (std::abs(target.x) < deadzone && std::abs(target.y) < deadzone) {
        return std::nullopt; //Goal reached, return empty optional
    }
    return target;
}

/**
 * @brief Callback function for the Load Service, used to load .mipi files and select which instrument channel
 * @param request The request from the service call, containing the filepath and instrument index
 * @param response The response for the service call, containing a message
 */
void Control::Controller::load_callback(const std::shared_ptr<jamc::srv::Load::Request> request, std::shared_ptr<jamc::srv::Load::Response> response)
{
    RCLCPP_INFO(this->get_logger(), "Received load request: filepath=%s, instrument_index=%d", request->filepath.c_str(), request->index);
    if (!connor.load_json_file(request->filepath)) {
        response->message = "[ERROR] Failed to load file: " + request->filepath;
        song_loaded_ = false;
        return;
    }
    int i = 0;
    RCLCPP_INFO(this->get_logger(), "Load Here %d", i++);
    std::lock_guard<std::mutex> lock(song_mutex_); // Ensure thread-safe access to song data
    play_ = false; //Reset play state when loading a new song
    state_ = STATE::MOVING; //Set state to moving to move to the first note position when a new song is loaded
    CONTROL_TIME = 0.0; //Reset control time for next time
    current_note_index_ = 0; //Reset note index when loading a new song
    song_loaded_ = true; //Mark that a song has been loaded
    RCLCPP_INFO(this->get_logger(), "Load Here %d", i++);
    song_ = connor.get_keyboard_indexs()[request->index];
    RCLCPP_INFO(this->get_logger(), "Load Here %d", i++);
    note_timings_ = connor.get_channel_note_timings()[request->index];
    RCLCPP_INFO(this->get_logger(), "Load Here %d", i++);
    note_durations_ = connor.get_channel_note_durations()[request->index];
    RCLCPP_INFO(this->get_logger(), "Load Here %d", i++);
    if (song_.empty() or song_.size() < 2) {
        response->message = "[ERROR] Loaded file but no (or 1) note(s) found for instrument index: " + std::to_string(request->index);
        song_loaded_ = false;
        return;
    }
    response->message = "[SUCCESS] Loaded file: " + request->filepath + " with instrument index: " + std::to_string(request->index);
}

/**
 * @brief Callback function for the TimeScale Service, used to set the time scaling factor for playback
 * @param request The request from the service call, containing the time scale factor
 * @param response The response for the service call, containing a message
 */
void Control::Controller::time_scale_callback(const std::shared_ptr<jamc::srv::TimeScale::Request> request, std::shared_ptr<jamc::srv::TimeScale::Response> response)
{
    std::lock_guard<std::mutex> lock(song_mutex_); //Ensure thread-safe access to time_scale_
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
    std::lock_guard<std::mutex> lock(song_mutex_); //Ensure thread-safe access to play_
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
{   
    std::lock_guard<std::mutex> lock(song_mutex_); //Ensure thread-safe access to direction_
    (void)request; // Unused parameter
    direction_ = !direction_;
    RCLCPP_INFO(this->get_logger(), "Toggled play direction. Now playing: %s", direction_ ? "forward" : "backward");
    response->message = std::string("[SUCCESS] Toggled play direction. Now playing: ") + (direction_ ? "forward" : "backward");
}

void Control::Controller::control_loop()
{
    int note = 0;
    double duration = 0.0;
    long timing = 0;

    bool dir = true;
    double time_scale = 1.0;
    {
        std::lock_guard<std::mutex> lock(song_mutex_);
        if(!play_ || !song_loaded_) {
            return; //If not playing or no song loaded, skip the control loop
        }
        if(current_note_index_ >= static_cast<int>(song_.size())) {
            play_ = false; //Stop playback if we've reached the end of the song
            current_note_index_ = 0; //Reset note index for next time
            state_ = STATE::MOVING;
            CONTROL_TIME = 0.0; //Reset control time for next time
            RCLCPP_INFO(this->get_logger(), "Reached end of song, stopping playback.");
            return;
        } else if (current_note_index_ < 0) {
            play_ = false; //Stop playback if we've reached the beginning of the song in reverse
            current_note_index_ = 0; //Reset note index for next time
            CONTROL_TIME = 0.0; //Reset control time for next time
            state_ = STATE::MOVING;
            RCLCPP_INFO(this->get_logger(), "Reached beginning of song, stopping playback.");
            return;
        }
        note = song_[current_note_index_];
        duration = note_durations_[current_note_index_];
        timing = static_cast<long>(note_timings_[current_note_index_]*1000);
        dir = direction_;
        time_scale = 1/time_scale_;
        if(time_scale < 0.05) time_scale = 0.05;
    }

    //State machine to handle timing of notes. 
    switch(state_)
    {
        case STATE::WAITING: {
            if(CONTROL_TIME >= timing * time_scale) { //If moving has already exceeded the time to reach the note, we go straight to playing, otherwise, wait until it is time
                state_ = STATE::PLAYING; //Go to playing state once we've reached the note timing
                CONTROL_TIME = 0.0; //Reset control time for the playing state
                sendStop();
            }
            break;
        }
        case STATE::PLAYING: {
            std::optional<vector3> target = play_note(duration * time_scale, CONTROL_TIME);
            if(!target.has_value()) {
                {
                    std::lock_guard<std::mutex> lock(song_mutex_);
                    if(dir) {
                        current_note_index_++;
                    } else {
                        current_note_index_--;
                    }
                }
                state_ = STATE::MOVING;
                CONTROL_TIME = 0; //Reset control time for the moving state
                sendStop();
            } else {
                sendVector(target.value());            
            }
            break;
        }
        case STATE::MOVING: {
            std::optional<vector3> target = calculate_velocity(note);
            if(!target.has_value()) {
                sendStop();
                state_ = STATE::WAITING; //Go to the waiting step once target position is reached
            } else {
                sendVector(target.value());
            }
            break;
        }
        default: {
            RCLCPP_ERROR(this->get_logger(), "Unknown state!");
        }
    }
    auto now = Clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - LAST_CONTROL_TIME_POINT
    ).count();

    CONTROL_TIME += delta;
    LAST_CONTROL_TIME_POINT = now;
}

// Need a play_note function that returns optional, normally would just give a z velocity back to be sent, when no value is received we would then increment the note and set state back to moving
/**
 * @brief Plays a single note or presses a button, streams Z velocity for a hardcoded duration to push down on whatever the EE is above
 * @param duration The time to hold the note for
 * @param time The current time into the note
 */
std::optional<vector3> Control::Controller::play_note(double duration, double time)
{
    //Will eventually do this, same time when we do Z velocity stuff
    return std::nullopt;
}

/**
 * @brief A helper method for calculating the target Z velocity (Scaled to meet a target height based on the X&Y magnitued)
 * @param xy The magnitude of the vector in the x and y direction
 */
double Control::Controller::calculate_z()
{
    // auto transform = tf_buffer_.lookupTransform("base_link", "tool0", tf2::TimePointZero); //Get the position of the end effector using TF
    //OR If we can get Z height from Ayberks april tags, that would work too and might be more accurate
    return 0;
}

void Control::Controller::key_positions_callback(const geometry_msgs::msg::PoseArray::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(key_positions_mutex_);
    if(msg->poses.empty()) return;
    if(msg->poses.size() != 37) { //FIND OUT WHAT ACTUAL NUMBER IS MEANT TO BE
        RCLCPP_WARN(this->get_logger(), "Received key positions but size is not 37. Received size: %zu", msg->poses.size());
    }
    key_positions_ = *msg;
}

// TODO:
/*
- Initialisation sequences to bring robot from home to the starting position for playing (Can probably get target joint positions from testing and just stream individual joint velocities to move there on startup)
    Topic: /servo_server/delta_joint_cmds (I believe)
    Message type: control_msgs::msg::JointJog (I think)
    Velocity in rad/s
- Implement state machine for handling note timings and transitions (Waiting, Moving, Pressing, etc)
- Implement actual Z velocity control for pressing keys based on a target Z height, read current EE Z Height from TF. Bring to hover above key until not timing is expired (or if the move took longer than the note timing) (probably minus a constant from not timing so that it plays in time)
*/
