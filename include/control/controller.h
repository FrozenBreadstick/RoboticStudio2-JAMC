#ifndef CONTROLLER_H
#define CONTROLLER_H

// Project Includes
#include "jamc/srv/load.hpp"
#include "jamc/srv/func.hpp"
#include "jamc/srv/time_scale.hpp"

#include "midi_processor/midi_processor.h"

// System path includes
#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <chrono>
#include <cmath>
#include <optional>
#include <geometry_msgs/msg/pose_array.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <control_msgs/msg/joint_jog.hpp>

using Clock = std::chrono::steady_clock;

namespace Control
{
    /**
     * @brief A struct for representing 3D vectors, used for velocity commands and target positions
     */
    struct vector3
    {
        double x;
        double y;
        double z;
    };

    /**
     * @brief An enum for representing the state of the controller, used to determine behavior in the control loop
     * WAITING: Waiting for the next note to play, no velocity commands are sent
     * PLAYING: Actively playing a note (Up and down z movement)
     * MOVING: Moving to the target position for the next note
     */
    enum class STATE
    {
        WAITING,
        PLAYING,
        MOVING
    };

    class Controller : public rclcpp::Node
    {
    public:
        Controller();
        ~Controller();
    private:
        //Variables & Helpers
        /**
         * @name Variables & Helpers
         * Variables used by the class
         */
        ///@{
        long CONTROL_TIME; //Variable for the control loop to use for timing
        Clock::time_point LAST_CONTROL_TIME_POINT; //The last time the control loop ran, used to calculate delta time for timing the notes
        STATE state_; //The current state of the controller, used to determine behavior in the control loop
        MidiProcessor connor;
        std::mutex key_positions_mutex_; //Mutex to protect access to the key positions
        geometry_msgs::msg::PoseArray key_positions_; //The positions of the keys on the piano, used for calculating target positions for the end effector

        geometry_msgs::msg::Point ee_; //The current end effector position
        std::mutex ee_mutex_; //Mutex to protect access to the end effector position

        double global_speed_scaling_ = 5.0; //A global speed scaler for the baseline speed scaling, seperate from the time scaling by the UI

        std::mutex song_mutex_; //Mutex to protect access to the song data
        std::vector<int> song_; //The notes in the currently loaded channel of the currently loaded song
        std::vector<double> note_timings_; //The timings of the notes in the current channel of the currently loaded song
        std::vector<double> note_durations_; //The durations of the notes in the current channel of the currently loaded song

        bool play_ = false; //Whether the song should be playing or not
        double time_scale_ = 1.0; //The time scaling factor for playback, default is 1.0 (normal speed)
        bool direction_ = true; //The direction of playback, true for forward, false for backward
        int current_note_index_ = 0; //The index of the current note being played in the song
        bool song_loaded_ = false; //Whether a song has been loaded or not

        std::mutex joint_mutex_; //Mutex to protect access to the latest joint state
        std::vector<double> latest_joint_state_; //The latest joint state of the robot

        rclcpp::TimerBase::SharedPtr control_timer_; //Timer for the control loop
        rclcpp::TimerBase::SharedPtr startup_timer_; //Timer for the startup sequence

        bool startup_complete_ = false; //Whether the startup sequence is complete or not

        double playnote_target_x = 0.0; //Hardcoded x offset to play a key on the keyboard
        double playnote_target_y = 0.015; //Hardcoded y offset to play a key on the keyboard
        double playnote_target_z = -0.015; //Hardcoded z offset to play a key on the keyboard
        geometry_msgs::msg::Point start_pn_ee_;
        geometry_msgs::msg::Point pn_target_;
        bool playing_note_ = false;
        bool returning_note_ = false;
        ///@}

        //Methods
        /**
         * @name General Methods
         * @brief Methods used throughout the class for various purposes
         */
        ///@{
        void sendTwistMsg(double x, double y, double z, double angular_x = 0.0, double angular_y = 0.0, double angular_z = 0.0);
        void sendJointJog(double shoulder_lift, double elbow, double wrist_1, double wrist_2, double wrist_3, double shoulder_pan);
        void sendJointJog(std::vector<double> vel);
        void sendVector(const vector3 &vec);
        void sendStop();
        int activeTrackDebug(vector3 target, bool x = false, bool y = false, bool z = false);
        ///@}

        //Startup & Shutdown
        /**
         * @name Startup & Shutdown Sequence
         * @brief Functions related to the startup and shutdown sequence of the controller
         */
        ///@{
        void startup();
        ///@}

        //Publishers
        /**
         * @name Publishers
         * @brief All Publishers that the Controller Class uses
         */
        ///@{
        rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;
        rclcpp::Publisher<control_msgs::msg::JointJog>::SharedPtr joint_traj_streaming_pub_;
        ///@}

        //Subscribers
        /**
         * @name Subscribers
         * @brief All Subscribers that the Controller Class uses
         */
        ///@{
        rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr debug_target_sub_;
        rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr key_positions_sub_;
        rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
        rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr ee_sub_;
        ///@}

        //Subscriber Callbacks
        /**
         * @name Subscriber Callbacks
         * @brief All Subscriber callbacks that the Controller Class uses
         */
        ///@{
        void debug_target_callback(const geometry_msgs::msg::Point::SharedPtr msg);
        void key_positions_callback(const geometry_msgs::msg::PoseArray::SharedPtr msg);
        void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg);
        void ee_callback(const geometry_msgs::msg::Point::SharedPtr msg);
        ///@}

        //Services
        /**
         * @name Services
         * @brief Services that the Controller Class provides
         */
        ///@{
        rclcpp::Service<jamc::srv::Load>::SharedPtr load_service_;
        rclcpp::Service<jamc::srv::TimeScale>::SharedPtr time_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr play_pause_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr play_direction_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr debug_service_;
        ///@}

        //Service Callbacks
        /**
         * @name Service Callbacks
         * @brief All Service callbacks that the Controller Class uses
         */
        ///@{
        void load_callback(const std::shared_ptr<jamc::srv::Load::Request> request, std::shared_ptr<jamc::srv::Load::Response> response);
        void time_scale_callback(const std::shared_ptr<jamc::srv::TimeScale::Request> request, std::shared_ptr<jamc::srv::TimeScale::Response> response);
        void play_pause_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response);
        void play_direction_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response);
        void debug_service_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response);
        ///@}

        //Control Loop
        /**
         * @name Control Loop
         * @brief Functions related to the control loop of the controller
         */
        ///@{
        void control_loop();
        std::optional<vector3> calculate_velocity(int note);
        std::optional<vector3> play_note(double duration, double time);
        ///@}
    };
}


#endif // CONTROLLER_H