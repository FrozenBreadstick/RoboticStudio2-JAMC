#ifndef UI_PIANO_H
#define UI_PIANO_H

#include "rclcpp/rclcpp.hpp"
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QFileDialog>
#include <QDir>

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
            void play_pause();
            
            QLabel* _label;
            QSpinBox* _input;
            QPushButton* _button;
            QPushButton* _file_button;  // New button
            QString _midi_file_path;    // Store selected path

            bool is_playing = false;

        private slots:
            void open_midi_file();      // Slot for file selection
    };
}

#endif // UI_PIANO_H