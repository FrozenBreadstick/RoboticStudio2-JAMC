#ifndef UI_PIANO_H
#define UI_PIANO_H

#include "rclcpp/rclcpp.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QFileDialog>
#include <QComboBox>
#include <QRadioButton>
#include <QDir>
#include <QString>
#include <QFileInfo>
#include <QtWidgets/QButtonGroup>

#include "jamc/srv/func.hpp"
#include "jamc/srv/time_scale.hpp"
#include "jamc/srv/load.hpp"

#include <sensor_msgs/msg/image.hpp>

#include "midi_processor/midi_processor.h"

namespace UI
{
    // Enum to track current direction state
    enum class PlaybackDirection { Forward, Reverse, None };

    class PianoUI : public QWidget, public rclcpp::Node
    {
        Q_OBJECT
        public:
            PianoUI();
            ~PianoUI();

        private:
            // State tracking
            bool is_playing = false;
            PlaybackDirection current_dir = PlaybackDirection::Forward; // Default to Forward
            QString _midi_file_path;

            // UI Elements
            QLabel* _title;
            QSlider* _track_slider;
            QPushButton* _play_pause_button;
            QPushButton* _direction_button; // Single toggle button for direction
            QLabel* _channel_title;
            QRadioButton* _channel_select;
            QSlider* _speed_control;
            QLabel* _new_file_label;
            QPushButton* _new_file_button;
            QLabel* _old_file_label;
            QComboBox* _old_file_button;
            
            // Status Labels
            QLabel* _status_val;
            QLabel* _speed_val;
            QLabel* _time_val;

            // Layouts
            QVBoxLayout* _channel_layout;
            QButtonGroup* _channel_group;

            void update_status_info();
            void force_pause_and_reset();
            void populate_mipi_combobox();
            void update_channel_radio_buttons();

            rclcpp::Client<jamc::srv::Func>::SharedPtr playback_client;
            rclcpp::Client<jamc::srv::Func>::SharedPtr direction_client;
            rclcpp::Client<jamc::srv::TimeScale>::SharedPtr time_scale_client;
            rclcpp::Client<jamc::srv::Load>::SharedPtr channel_client;

            MidiProcessor processor;

            QLabel* _camera_view;
            rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr _camera_sub;

            QPushButton* _debug_button;

            void image_callback(const sensor_msgs::msg::Image::SharedPtr msg);

        private slots:
            void play_pause();
            void open_midi_file();
            void toggle_direction(); // New toggle slot
            void set_direction_forward();
            void set_direction_reverse();
            void send_time_scale();
            void send_channel_selection(int index);
            void load_existing_mipi(int index);
            void send_debug_request();
    };
};

#endif // UI_PIANO_H