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

using nlohmann::json;

namespace Control
{
    struct vector3
    {
        double x;
        double y;
        double z;
    };

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

        void sendVector(const vector3 &vec);
        void sendStop();

        int activeTrackDebug(vector3 target, bool x = false, bool y = false, bool z = false);

    private:
        //Variables & Helpers
        STATE state_ = STATE::WAITING; //The current state of the controller, used to determine behavior in the control loop
        MidiProcessor connor;
        std::mutex key_positions_mutex_; //Mutex to protect access to the key positions
        geometry_msgs::msg::PoseArray key_positions_; //The positions of the keys on the piano, used for calculating target positions for the end effector

        std::mutex song_mutex_; //Mutex to protect access to the song data
        std::vector<int> song_; //The notes in the currently loaded channel of the currently loaded song
        std::vector<double> note_timings_; //The timings of the notes in the current channel of the currently loaded song
        std::vector<double> note_durations_; //The durations of the notes in the current channel of the currently loaded song

        bool play_ = false; //Whether the song should be playing or not
        double time_scale_ = 1.0; //The time scaling factor for playback, default is 1.0 (normal speed)
        bool direction_ = true; //The direction of playback, true for forward, false for backward
        int current_note_index_ = 0; //The index of the current note being played in the song
        bool song_loaded_ = false; //Whether a song has been loaded or not

        //Methods
        void sendTwistMsg(double x, double y, double z, double angular_x = 0.0, double angular_y = 0.0, double angular_z = 0.0);

        //Publishers
        rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;

        //Subscribers
        rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr debug_target_sub_;
        void debug_target_callback(const geometry_msgs::msg::Point::SharedPtr msg);
        rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr key_positions_sub_;
        void key_positions_callback(const geometry_msgs::msg::PoseArray::SharedPtr msg);

        //Services
        /**
         * @name ROS2 Services
         * Services that the Controller Class provides
         */
        ///@{
        rclcpp::Service<jamc::srv::Load>::SharedPtr load_service_;
        rclcpp::Service<jamc::srv::TimeScale>::SharedPtr time_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr play_pause_service_;
        rclcpp::Service<jamc::srv::Func>::SharedPtr play_direction_service_;
        ///@}
        void load_callback(const std::shared_ptr<jamc::srv::Load::Request> request, std::shared_ptr<jamc::srv::Load::Response> response);
        void time_scale_callback(const std::shared_ptr<jamc::srv::TimeScale::Request> request, std::shared_ptr<jamc::srv::TimeScale::Response> response);
        void play_pause_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response);
        void play_direction_callback(const std::shared_ptr<jamc::srv::Func::Request> request, std::shared_ptr<jamc::srv::Func::Response> response);

        //Timers
        rclcpp::TimerBase::SharedPtr control_timer_;
        void control_loop();
        double calculate_z();
        std::optional<vector3> calculate_velocity(int note);

    };
}


#endif // CONTROLLER_H