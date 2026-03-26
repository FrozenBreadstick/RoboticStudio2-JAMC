#ifndef UI_PIANO_H
#define UI_PIANO_H

#include "rclcpp/rclcpp.hpp"
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
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

namespace UI
{
    class PianoUI : public QWidget, public rclcpp::Node
    {
        Q_OBJECT
        public:
            PianoUI();
            ~PianoUI();

        private:
            void send_request();
            
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
            QString _midi_file_path;

            bool is_playing = false;

        private slots:
            void play_pause();
            void open_midi_file();      
    };
}

#endif // UI_PIANO_H