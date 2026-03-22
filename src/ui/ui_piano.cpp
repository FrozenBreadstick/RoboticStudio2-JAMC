#include "ui/ui_piano.h"

namespace UI
{
    /*
    creation of the UI is done in the constructor, and all button actions are connected to their respective slots here as well.
    */
    PianoUI::PianoUI() : QWidget(), Node("piano_ui") {
        /*
        A dark theme stylesheet is defined here for easy access and modification. It is applied to the entire UI in the constructor.
        */
        this->setStyleSheet(
            "QWidget { background-color: #1e1e1e; color: #e0e0e0; font-family: 'Segoe UI', Arial, sans-serif; font-size: 14px; }"
            "QLabel#MainTitle { font-size: 28px; font-weight: bold; color: #00aaff; margin-top: 10px; }"
            "QGroupBox { font-weight: bold; border: 1px solid #3a3a3a; border-radius: 8px; margin-top: 20px; background-color: #252526; }"
            "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 10px; color: #00aaff; }"
            "QPushButton { background-color: #007acc; color: white; border: none; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
            "QPushButton:hover { background-color: #0098ff; }"
            "QPushButton:pressed { background-color: #005c99; }"
            "QComboBox, QRadioButton { color: #e0e0e0; }"
            "QSlider::groove:horizontal { border: 1px solid #3a3a3a; height: 8px; background: #1e1e1e; margin: 2px 0; border-radius: 4px; }"
            "QSlider::handle:horizontal { background: #00aaff; border: 1px solid #007acc; width: 16px; margin: -4px 0; border-radius: 8px; }"
        );

        RCLCPP_INFO(this->get_logger(), "Piano UI Started");

        // Main Layout setup
        auto* main_layout = new QVBoxLayout(this);
        
        // Title setup
        _title = new QLabel("MIDI To UR3", this);
        _title->setObjectName("MainTitle");
        _title->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(_title);
        
        main_layout->addStretch(1);

        // CONFIGURATION GROUP (Channels, Files, Speed)
        auto* config_group = new QGroupBox("Configuration", this);
        auto* config_layout = new QHBoxLayout(config_group);

        // Left: Channel Selector
        auto* channel_selector_layout = new QVBoxLayout();
        _channel_title = new QLabel("Channel Selector:", this);
        _channel_select = new QRadioButton("Channel 1", this);
        channel_selector_layout->addWidget(_channel_title);
        channel_selector_layout->addWidget(_channel_select);
        channel_selector_layout->setAlignment(Qt::AlignCenter);

        // Middle: File Selection
        auto* files_layout = new QVBoxLayout();
        _new_file_label = new QLabel("No File Selected", this);
        _new_file_label->setAlignment(Qt::AlignCenter);
        _new_file_button = new QPushButton("Select A MIDI File", this); 
        _old_file_label = new QLabel("Recent Files", this);
        _old_file_label->setAlignment(Qt::AlignCenter);
        _old_file_button = new QComboBox(this);

        connect(_new_file_button, &QPushButton::clicked, this, &PianoUI::open_midi_file);

        files_layout->addWidget(_new_file_label);
        files_layout->addWidget(_new_file_button);
        files_layout->addSpacing(15);
        files_layout->addWidget(_old_file_label);
        files_layout->addWidget(_old_file_button);
        files_layout->setAlignment(Qt::AlignCenter);

        // Right: Speed Control
        auto* speed_layout = new QVBoxLayout();
        QLabel* speed_label = new QLabel("Speed", this);
        speed_label->setAlignment(Qt::AlignCenter);
        _speed_control = new QSlider(Qt::Vertical, this);
        _speed_control->setMinimumHeight(100); // Stop it from stretching forever
        
        // Center the slider horizontally within its column
        auto* center_slider_layout = new QHBoxLayout();
        center_slider_layout->addStretch();
        center_slider_layout->addWidget(_speed_control);
        center_slider_layout->addStretch();

        speed_layout->addWidget(speed_label);
        speed_layout->addLayout(center_slider_layout);
        speed_layout->setAlignment(Qt::AlignCenter);

        // Create A Configuration Group
        config_layout->addStretch(); 
        config_layout->addLayout(channel_selector_layout);
        config_layout->addSpacing(40);
        config_layout->addLayout(files_layout);
        config_layout->addSpacing(40);
        config_layout->addLayout(speed_layout);
        config_layout->addStretch(); 

        main_layout->addWidget(config_group);
        main_layout->addSpacing(20);


        // PLAYBACK GROUP (Buttons, Track Slider)
        auto* playback_group = new QGroupBox("Playback", this);
        auto* playback_layout = new QVBoxLayout(playback_group);
        
        // Playback Buttons
        auto* play_forward_backward = new QHBoxLayout();
        _reverse_button = new QPushButton("⏪", this);
        _play_pause_button = new QPushButton("▶", this); 
        _forward_button = new QPushButton("⏩", this);

        play_forward_backward->addStretch(); 
        play_forward_backward->addWidget(_reverse_button);
        play_forward_backward->addWidget(_play_pause_button);
        play_forward_backward->addWidget(_forward_button);
        play_forward_backward->addStretch(); 

        connect(_play_pause_button, &QPushButton::clicked, this, &PianoUI::play_pause);

        // Slider
        _track_slider = new QSlider(Qt::Horizontal, this);
        _track_slider->setRange(0, 100); 
        _track_slider->setValue(0); 

        // Create A Playback Group
        playback_layout->addSpacing(10);
        playback_layout->addLayout(play_forward_backward); 
        playback_layout->addSpacing(15);
        playback_layout->addWidget(_track_slider);
        playback_layout->addSpacing(10);

        main_layout->addWidget(playback_group);

        main_layout->addStretch(2); 
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
            _play_pause_button->setText("▐▐");
            RCLCPP_INFO(this->get_logger(), "MIDI file Playing | Press to Pause");
        } else {
            _play_pause_button->setText("▶");
            RCLCPP_INFO(this->get_logger(), "MIDI file Paused | Press to Play");
        }
    }

    void PianoUI::open_midi_file() {
        QString file_name = QFileDialog::getOpenFileName(
            this,
            "Select MIDI File",
            QDir::homePath(),           
            "MIDI Files (*.mid *.midi)" 
        );

        if (!file_name.isEmpty()) {
            _midi_file_path = file_name;
            // Shorten the display name so it doesn't break the layout if the path is huge
            QFileInfo fileInfo(_midi_file_path);
            _new_file_label->setText(fileInfo.fileName()); 
            
            RCLCPP_INFO(this->get_logger(), "MIDI file selected: %s", file_name.toStdString().c_str());
        }
    }
}