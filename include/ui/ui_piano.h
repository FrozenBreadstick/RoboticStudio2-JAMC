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
            PlaybackDirection current_dir = PlaybackDirection::None;
            QString _midi_file_path;

            // UI Elements
            QLabel* _title;
            QSlider* _track_slider;
            QPushButton* _play_pause_button;
            QPushButton* _forward_button;
            QPushButton* _reverse_button;
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

            void update_status_info();

        private slots:
            void play_pause();
            void open_midi_file();
            void set_direction_forward();
            void set_direction_reverse();
    };
}

#endif // UI_PIANO_H