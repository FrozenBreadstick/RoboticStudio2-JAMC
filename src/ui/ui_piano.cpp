/**
 * @file ui_piano.cpp
 * @brief Implementation of the PianoUI class, providing a graphical interface for MIDI playback control.
 */

#include "ui/ui_piano.h"
#include <QList>
#include <chrono>

namespace UI
{
    /**
     * @brief Hybrid Qt-ROS2 Class for managing the User Interface of the MIDI-to-UR3 system. 
     * * @details This class inherits from QWidget and rclcpp::Node. It contains the following ROS2 I/O:
     * - **Service Clients**
     * - `/MIPI/play_pause` (jamc/srv/Func): Triggers a play/pause state change on the robot.
     * - `/MIPI/direction` (jamc/srv/Func): Toggles the playback direction (Forward/Reverse).
     * - `/MIPI/time_scale` (jamc/srv/TimeScale): Sends a float (0.01 to 1.0) to scale robot velocity.
     * - `/MIPI/load` (jamc/srv/Load): Sends the filepath and instrument index to the controller.
     * - **Subscriptions**
     * - `/camera/camera/color/image_raw` (sensor_msgs/msg/Image): Receives live feed for UI visualization.
     */
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

        processor = MidiProcessor();

        // Client Creations
        playback_client = this->create_client<jamc::srv::Func>("/MIPI/play_pause");
        direction_client = this->create_client<jamc::srv::Func>("/MIPI/direction");
        time_scale_client = this->create_client<jamc::srv::TimeScale>("/MIPI/time_scale");
        channel_client = this->create_client<jamc::srv::Load>("/MIPI/load");
        debug_client = this->create_client<jamc::srv::Func>("/MIPI/debug");

        // Camera Subscription
        _camera_sub = this->create_subscription<sensor_msgs::msg::Image>
        ("/camera/camera/color/image_raw", 10, std::bind(&PianoUI::image_callback, this, std::placeholders::_1));

        auto* main_layout = new QVBoxLayout(this);
        
        _title = new QLabel("MIDI To UR3", this);
        _title->setObjectName("MainTitle");
        _title->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(_title);
        main_layout->addStretch(1);

        // --- CAMERA VISUALISATION GROUP ---
        _camera_view = new QLabel("Waiting for camera feed...", this);
        _camera_view->setFixedSize(400, 300);
        _camera_view->setAlignment(Qt::AlignCenter);
        _camera_view->setStyleSheet("border: 2px solid #00aaff; background-color: black; border-radius: 5px; color: #00aaff;");
        main_layout->addWidget(_camera_view, 0, Qt::AlignCenter);

        // --- CONFIGURATION GROUP ---
        auto* config_group = new QGroupBox("Configuration", this);
        auto* config_layout = new QHBoxLayout(config_group);

        _channel_layout = new QVBoxLayout(); 
        _channel_title = new QLabel("Channel Selector:", this);
        _channel_layout->addWidget(_channel_title);

        _channel_group = new QButtonGroup(this);
        connect(_channel_group, QOverload<int>::of(&QButtonGroup::idClicked), this, &PianoUI::send_channel_selection);

        auto* files_layout = new QVBoxLayout();
        _new_file_label = new QLabel("No File Selected", this);
        _new_file_button = new QPushButton("Select A MIDI File", this); 
        _old_file_button = new QComboBox(this);
        connect(_old_file_button, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PianoUI::load_existing_mipi);

        files_layout->addWidget(_new_file_label);
        files_layout->addWidget(_new_file_button);
        files_layout->addWidget(_old_file_button);

        _debug_button = new QPushButton("🐛", this);

        auto* speed_layout = new QVBoxLayout();
        speed_layout->addWidget(new QLabel("Speed", this), 0, Qt::AlignCenter);
        _speed_control = new QSlider(Qt::Vertical, this);
        _speed_control->setRange(1, 100); 
        _speed_control->setValue(100);    
        _speed_control->setMinimumHeight(100);
        speed_layout->addWidget(_speed_control, 0, Qt::AlignCenter);

        config_layout->addLayout(_channel_layout);
        config_layout->addLayout(files_layout);
        config_layout->addLayout(speed_layout);
        main_layout->addWidget(config_group);

        // --- PLAYBACK GROUP ---
        auto* playback_group = new QGroupBox("Playback", this);
        auto* playback_layout = new QVBoxLayout(playback_group);
        
        auto* btn_layout = new QHBoxLayout();
        _direction_button = new QPushButton("⏩", this); 
        _play_pause_button = new QPushButton("▶", this); 
        btn_layout->addWidget(_direction_button);
        btn_layout->addWidget(_play_pause_button);

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
        connect(_direction_button, &QPushButton::clicked, this, &PianoUI::toggle_direction); 
        connect(_speed_control, &QSlider::valueChanged, this, &PianoUI::send_time_scale);
        connect(_track_slider, &QSlider::valueChanged, this, &PianoUI::update_status_info);
        connect(_debug_button, &QPushButton::clicked, this, &PianoUI::send_debug_request);

        main_layout->addStretch(2);
        
        populate_mipi_combobox();
        update_status_info();
    }

    /**
     * @brief Standard Destructor for PianoUI Class.
     */
    PianoUI::~PianoUI() {}

    /**
     * @brief Callback function for processing incoming ROS 2 image messages.
     * @param msg A shared pointer to the incoming sensor_msgs::msg::Image.
     * @note This uses QMetaObject::invokeMethod to ensure the UI update happens on the main thread, avoiding thread collisions with the ROS executor.
     */
    void PianoUI::image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        static auto last_frame_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_frame_time).count() < 50) {
            return;
        }

        last_frame_time = current_time;

        if (msg->encoding == "rgb8") {
            QImage open_img(&msg->data[0], msg->width, msg->height, msg->step, QImage::Format_RGB888);
            QImage clean_img = open_img.mirrored(true, false);
            QPixmap pixmap = QPixmap::fromImage(clean_img).scaled(_camera_view->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

            QMetaObject::invokeMethod(_camera_view, [this, pixmap]() {
                _camera_view->setPixmap(pixmap);
            }, Qt::QueuedConnection);
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                "PianoUI Camera Warning: Expected 'rgb8' image encoding, but received '%s'", msg->encoding.c_str());
        }
    }

    /**
     * @brief Sends a trigger request to the debug service for development testing.
     */
    void PianoUI::send_debug_request() {
        auto request = std::make_shared<jamc::srv::Func::Request>();
        RCLCPP_INFO(this->get_logger(), "Sending DEBUG request");

        if (!debug_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Playback service is not available for debug.");
            return;
        }

        debug_client->async_send_request(request, 
            [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                try {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Debug response received: %s", response->message.c_str());
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Debug service call failed: %s", e.what());
                }
            });
    }

    /**
     * @brief Immediate helper method to halt UI playback tracking and reset visual sliders.
     * @details Sends a PAUSE request to the robot and resets the track slider to zero. Always enforces a Forward direction state.
     */
    void PianoUI::force_pause_and_reset() {
        if (is_playing) {
            is_playing = false;
            _play_pause_button->setText("▶");
            
            auto request = std::make_shared<jamc::srv::Func::Request>();
            if (playback_client->service_is_ready()) {
                RCLCPP_INFO(this->get_logger(), "Sending PAUSE request to reset playback");
                playback_client->async_send_request(request);
            }
        }
        _track_slider->setValue(0);
        set_direction_forward();
    }

    /**
     * @brief Scans the ~/mipi_files directory to refresh the dropdown list.
     * @note Blocks signals during execution to prevent recursive load triggers.
     */
    void PianoUI::populate_mipi_combobox() {
        _old_file_button->blockSignals(true);
        _old_file_button->clear();
        _old_file_button->addItem("Load an existing .mipi file...");
        
        QDir mipi_dir(QDir::homePath() + "/mipi_files");
        if (mipi_dir.exists()) {
            QStringList files = mipi_dir.entryList(QStringList() << "*.mipi", QDir::Files);
            for (const QString& f : files) {
                _old_file_button->addItem(f);
            }
        }
        _old_file_button->blockSignals(false);
    }

    /**
     * @brief Rebuilds the radio button group based on the currently loaded MIDI processor data.
     */
    void PianoUI::update_channel_radio_buttons() {
        QList<QAbstractButton*> buttons = _channel_group->buttons();
        for (QAbstractButton* btn : buttons) {
            _channel_group->removeButton(btn);
            _channel_layout->removeWidget(btn);
            btn->deleteLater();
        }

        std::vector<std::string> instrument_list = processor.get_instrument_names();
        for (size_t i = 0; i < instrument_list.size(); ++i) {
            QRadioButton* rb = new QRadioButton(QString::fromStdString(instrument_list[i]), this);
            _channel_layout->addWidget(rb);
            _channel_group->addButton(rb, static_cast<int>(i)); 
        }
        
        if (!instrument_list.empty()) {
            _channel_group->button(0)->setChecked(true);
        }
    }

    /**
     * @brief Toggles the playback state and notifies the /MIPI/play_pause service.
     */
    void PianoUI::play_pause(){
        is_playing = !is_playing;
        _play_pause_button->setText(is_playing ? "▐▐" : "▶");
        
        auto request = std::make_shared<jamc::srv::Func::Request>();
        if (!playback_client->service_is_ready()) return;

        playback_client->async_send_request(request, 
            [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
            try {
                auto response = future.get();
                RCLCPP_INFO(this->get_logger(), "Response: %s", response->message.c_str());
            } catch (const std::exception &e) {
                RCLCPP_ERROR(this->get_logger(), "Playback service failed: %s", e.what());
            }
        });
    }

    /**
     * @brief UI Slot for opening a file dialog to process a raw .mid file.
     * @details Converts the file to .mipi and automatically selects the first available channel for loading.
     */
    void PianoUI::open_midi_file() {
        QString file_name = QFileDialog::getOpenFileName(this, "Select MIDI File", QDir::homePath(), "MIDI Files (*.mid *.midi)");        
        if (file_name.isEmpty()) return;

        std::string std_midi_path = file_name.toStdString();
        QString json_name = QFileInfo(file_name).baseName() + ".mipi";
        std::string std_json_name = json_name.toStdString();

        if (processor.processMidiFile(std_midi_path, std_json_name) == 0) {
            _midi_file_path = QDir::homePath() + "/mipi_files/" + json_name;
            _new_file_label->setText(QFileInfo(file_name).fileName());

            update_channel_radio_buttons();
            populate_mipi_combobox(); 
            _track_slider->setRange(0, static_cast<int>(processor.get_song_duration()));
            update_status_info();
            
            if (!processor.get_channels().empty()) send_channel_selection(0);
        }
    }

    /**
     * @brief UI Slot for loading a file selected from the dropdown menu.
     * @param index The index of the selected file in the QComboBox.
     */
    void PianoUI::load_existing_mipi(int index) {
        if (index <= 0) return;

        QString mipi_filename = _old_file_button->itemText(index);
        if (processor.load_json_file(mipi_filename.toStdString())) {
            _midi_file_path = QDir::homePath() + "/mipi_files/" + mipi_filename;
            _new_file_label->setText(mipi_filename);
            update_channel_radio_buttons();
            _track_slider->setRange(0, static_cast<int>(processor.get_song_duration()));
            update_status_info();

            if (!processor.get_channels().empty()) send_channel_selection(0);
        }
    }

    /**
     * @brief Sends a load request to the robot controller for a specific instrument channel.
     * @param button_id The ID of the radio button corresponding to the instrument index.
     * @details Clears the current playback state before sending the new file data to the robot.
     */
    void PianoUI::send_channel_selection(int button_id) {
        if (button_id < 0 || _midi_file_path.isEmpty()) return;

        force_pause_and_reset();

        std::vector<int> channels = processor.get_channels();
        if (button_id >= static_cast<int>(channels.size())) return;
        int selected_channel = channels.at(button_id);

        auto request = std::make_shared<jamc::srv::Load::Request>();
        request->filepath = (QFileInfo(_midi_file_path).baseName() + ".mipi").toStdString();
        request->index = selected_channel; 

        if (!channel_client->service_is_ready()) return;

        channel_client->async_send_request(request, 
            [this](rclcpp::Client<jamc::srv::Load>::SharedFuture future) {
                try {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Load response received: %s", response->message.c_str());
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Load service call failed: %s", e.what());
                }
            });
    }

    /**
     * @brief Toggles between Forward and Reverse playback logic.
     */
    void PianoUI::toggle_direction() {
        if (current_dir == PlaybackDirection::Forward) set_direction_reverse();
        else set_direction_forward();
    }

    /**
     * @brief Explicitly sets playback direction to Forward via the /MIPI/direction service.
     */
    void PianoUI::set_direction_forward() {
        if (current_dir == PlaybackDirection::Forward) return;
        current_dir = PlaybackDirection::Forward;
        _direction_button->setText("⏩");

        auto request = std::make_shared<jamc::srv::Func::Request>();
        if (!direction_client->service_is_ready()) return;
        direction_client->async_send_request(request);
    }

    /**
     * @brief Explicitly sets playback direction to Reverse via the /MIPI/direction service.
     */
    void PianoUI::set_direction_reverse() {
        if (current_dir == PlaybackDirection::Reverse) return;
        current_dir = PlaybackDirection::Reverse;
        _direction_button->setText("⏪");

        auto request = std::make_shared<jamc::srv::Func::Request>();
        if (!direction_client->service_is_ready()) return;
        direction_client->async_send_request(request);
    }

    /**
     * @brief Refreshes all status labels in the UI with the latest system state information.
     */
    void PianoUI::update_status_info() {
        _status_val->setText(_midi_file_path.isEmpty() ? "Status: No File" : "Status: Ready");
        int speed_percent = _speed_control->value();
        _speed_val->setText(QString("Speed: %1%").arg(speed_percent));
        _time_val->setText(QString("Time: %1 / %2").arg(_track_slider->value()).arg(processor.get_song_duration()));
    }

    /**
     * @brief Transmits the current speed slider value as a time scale factor (0.01-1.0) to the robot.
     */
    void PianoUI::send_time_scale() {
        update_status_info();
        auto request = std::make_shared<jamc::srv::TimeScale::Request>();
        request->scale = static_cast<double>(_speed_control->value()) / 100.0;
        
        if (!time_scale_client->service_is_ready()) return;
        time_scale_client->async_send_request(request);
    }
}