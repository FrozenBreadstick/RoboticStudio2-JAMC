#include "ui/ui_piano.h"

namespace UI
{
    PianoUI::PianoUI() : QWidget(), Node("piano_ui") {
        this->setStyleSheet(
            "QWidget { background-color: #1e1e1e; color: #e0e0e0; font-family: 'Segoe UI', Arial, sans-serif; font-size: 14px; }"
            "QLabel#MainTitle { font-size: 28px; font-weight: bold; color: #00aaff; margin-top: 10px; }"
            "QGroupBox { font-weight: bold; border: 1px solid #3a3a3a; border-radius: 8px; margin-top: 20px; background-color: #252526; }"
            "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 10px; color: #00aaff; }"
            "QPushButton { background-color: #007acc; color: white; border: none; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
            "QPushButton:hover { background-color: #0098ff; }"
            "QPushButton:pressed { background-color: #005c99; }"
            "QSlider::groove:horizontal { border: 1px solid #3a3a3a; height: 8px; background: #1e1e1e; border-radius: 4px; }"
            "QSlider::handle:horizontal { background: #00aaff; border: 1px solid #007acc; width: 16px; margin: -4px 0; border-radius: 8px; }"
        );

        //Client Creations
            playback_client = this->create_client<jamc::srv::Func>("playback_control");
            direction_client = this->create_client<jamc::srv::Func>("playback_direction");
            time_scale_client = this->create_client<jamc::srv::TimeScale>("time_scale_control");
            channel_client = this->create_client<jamc::srv::Load>("channel_select");


        auto* main_layout = new QVBoxLayout(this);
        
        _title = new QLabel("MIDI To UR3", this);
        _title->setObjectName("MainTitle");
        _title->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(_title);
        main_layout->addStretch(1);

        // --- CONFIGURATION GROUP ---
        auto* config_group = new QGroupBox("Configuration", this);
        auto* config_layout = new QHBoxLayout(config_group);

        auto* channel_layout = new QVBoxLayout();
        _channel_title = new QLabel("Channel Selector:", this);
        _channel_select = new QRadioButton("Channel 1", this);
        channel_layout->addWidget(_channel_title);
        channel_layout->addWidget(_channel_select);

        auto* files_layout = new QVBoxLayout();
        _new_file_label = new QLabel("No File Selected", this);
        _new_file_button = new QPushButton("Select A MIDI File", this); 
        _old_file_button = new QComboBox(this);
        files_layout->addWidget(_new_file_label);
        files_layout->addWidget(_new_file_button);
        files_layout->addWidget(_old_file_button);

        auto* speed_layout = new QVBoxLayout();
        speed_layout->addWidget(new QLabel("Speed", this), 0, Qt::AlignCenter);
        _speed_control = new QSlider(Qt::Vertical, this);
        _speed_control->setRange(1, 100); // Set range from 1% to 100%
        _speed_control->setValue(100);    // Default to 100% (full speed)
        _speed_control->setMinimumHeight(100);
        speed_layout->addWidget(_speed_control, 0, Qt::AlignCenter);

        config_layout->addLayout(channel_layout);
        config_layout->addLayout(files_layout);
        config_layout->addLayout(speed_layout);
        main_layout->addWidget(config_group);

        // --- PLAYBACK GROUP ---
        auto* playback_group = new QGroupBox("Playback", this);
        auto* playback_layout = new QVBoxLayout(playback_group);
        
        auto* btn_layout = new QHBoxLayout();
        _reverse_button = new QPushButton("⏪", this);
        _play_pause_button = new QPushButton("▶", this); 
        _forward_button = new QPushButton("⏩", this);
        btn_layout->addWidget(_reverse_button);
        btn_layout->addWidget(_play_pause_button);
        btn_layout->addWidget(_forward_button);

        _track_slider = new QSlider(Qt::Horizontal, this);
        playback_layout->addLayout(btn_layout);
        playback_layout->addWidget(_track_slider);
        main_layout->addWidget(playback_group);

        // --- STATUS GROUP ---
        auto* status_group = new QGroupBox("System Status", this);
        auto* status_layout = new QHBoxLayout(status_group);
        _status_val = new QLabel("Status: Idle", this);
        _speed_val = new QLabel("Speed: 1.0x", this);
        _time_val = new QLabel("Time: 0/0", this);
        
        status_layout->addWidget(_status_val);
        status_layout->addStretch();
        status_layout->addWidget(_speed_val);
        status_layout->addStretch();
        status_layout->addWidget(_time_val);
        main_layout->addWidget(status_group);

        // --- CONNECTIONS ---
        connect(_new_file_button, &QPushButton::clicked, this, &PianoUI::open_midi_file);
        connect(_play_pause_button, &QPushButton::clicked, this, &PianoUI::play_pause);
        connect(_forward_button, &QPushButton::clicked, this, &PianoUI::set_direction_forward);
        connect(_reverse_button, &QPushButton::clicked, this, &PianoUI::set_direction_reverse);
        connect(_speed_control, &QSlider::valueChanged, this, &PianoUI::update_status_info);
        connect(_track_slider, &QSlider::valueChanged, this, &PianoUI::update_status_info);

        main_layout->addStretch(2);
        update_status_info(); // Initialize text labels
    }

    PianoUI::~PianoUI() {}

    void PianoUI::play_pause(){
        is_playing = !is_playing;
        _play_pause_button->setText(is_playing ? "▐▐" : "▶");
            // Create and send playback control request
            if (is_playing){
                auto request = std::make_shared<jamc::srv::Func::Request>();
                RCLCPP_INFO(this->get_logger(), "Sending PLAY request");

                //check service availability before sending request to avoid blocking the UI
                if (!playback_client->service_is_ready()) {
                    RCLCPP_WARN(this->get_logger(), "Playback service is not available.");
                    return;
                }

                playback_client->async_send_request(request, 
                    [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Response: %s", response->message.c_str());
                });
            }
            else {
                auto request = std::make_shared<jamc::srv::Func::Request>();
                RCLCPP_INFO(this->get_logger(), "Sending PAUSE request");
                
                //check service availability before sending request to avoid blocking the UI
                if (!playback_client->service_is_ready()) {
                    RCLCPP_WARN(this->get_logger(), "Playback service is not available.");
                    return;
                }

                playback_client->async_send_request(request, 
                    [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Response: %s", response->message.c_str());
                });
            }
    }

    void PianoUI::open_midi_file() {
        QString file_name = QFileDialog::getOpenFileName(this, "Select MIDI", QDir::homePath(), "MIDI Files (*.mid *.midi)");
        if (!file_name.isEmpty()) {
            _midi_file_path = file_name;
            _new_file_label->setText(QFileInfo(file_name).fileName());
            update_status_info(); // Trigger update on load
        }
    }

    void PianoUI::set_direction_forward() {
        if (current_dir != PlaybackDirection::Forward)
            {return;} // Prevent redundant requests if already in desired state

            // Create and send direction control request
            auto request = std::make_shared<jamc::srv::Func::Request>();
            RCLCPP_INFO(this->get_logger(), "Sending FORWARD direction request");
            
            //check service availability before sending request to avoid blocking the UI
            if (!direction_client->service_is_ready()) {
                RCLCPP_WARN(this->get_logger(), "Direction service is not available.");
                return;
            }

            direction_client->async_send_request(request, 
                [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Direction response received: %s", response->message.c_str());
                });
    }

    void PianoUI::set_direction_reverse() {
        if (current_dir != PlaybackDirection::Reverse)
            {return;} // Prevent redundant requests if already in desired state

            // Create and send direction control request
            auto request = std::make_shared<jamc::srv::Func::Request>();
            RCLCPP_INFO(this->get_logger(), "Sending REVERSE direction request");
            
            //check service availability before sending request to avoid blocking the UI
            if (!direction_client->service_is_ready()) {
                RCLCPP_WARN(this->get_logger(), "Direction service is not available.");
                return;
            }

            direction_client->async_send_request(request, 
                [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Direction response received: %s", response->message.c_str());
                });
    }

    void PianoUI::update_status_info() {
        _status_val->setText(_midi_file_path.isEmpty() ? "Status: No File" : "Status: Ready");
        
        int speed_percent = _speed_control->value();
        _speed_val->setText(QString("Speed: %1%").arg(speed_percent));

        // Time Display
        _time_val->setText(QString("Time: %1 / %2")
            .arg(_track_slider->value())
            .arg(_track_slider->maximum()));
    }
}