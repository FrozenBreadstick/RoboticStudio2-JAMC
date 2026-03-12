#include "ui_piano.h"

namespace UI
{
    PianoUI::PianoUI() : QWidget(), Node("piano_ui") {

        RCLCPP_INFO(this->get_logger(), "Piano UI Started");

        auto* layout = new QVBoxLayout(this);
        _label = new QLabel("", this);
        _input = new QSpinBox(this);
        _button = new QPushButton("▷", this);

        layout->addWidget(_label);
        layout->addWidget(_input);
        layout->addWidget(_button);

        connect(_button, &QPushButton::clicked, this, &PianoUI::play_pause);
    }

    PianoUI::~PianoUI() {
        RCLCPP_INFO(this->get_logger(), "Piano UI Stopped");
    }

    void PianoUI::send_request() {
        RCLCPP_INFO(this->get_logger(), "Button Pressed");
    }

    void PianoUI::play_pause(){
        is_playing = !is_playing;
        if(is_playing){
            _button->setText("▷");
            RCLCPP_INFO(this->get_logger(), "MIDI file paused");
        } else {
            _button->setText("▐▐");
            RCLCPP_INFO(this->get_logger(), "MIDI file playing");
        }

    void PianoUI::open_midi_file() {
    QString file_name = QFileDialog::getOpenFileName(
        this,
        "Select MIDI File",
        QDir::homePath(),           // Start in home directory
        "MIDI Files (*.mid *.midi)" // Filter
    );

    if (!file_name.isEmpty()) {
        _midi_file_path = file_name;
        _label->setText("Selected: " + _midi_file_path);
        RCLCPP_INFO(this->get_logger(), "MIDI file selected: %s", file_name.toStdString().c_str());
    }
}
    }
}
