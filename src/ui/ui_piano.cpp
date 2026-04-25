/**
 * @file ui_piano.cpp
 * @brief Implementation of the PianoUI class, providing a graphical interface for MIDI playback control.
 */

#include "ui/ui_piano.h"
#include <QList>

namespace UI
{
    /**
     * @brief Constructor for the PianoUI class.
     * * Initializes the Qt application window, sets up the stylesheet, creates ROS 2 clients 
     * for communication with the playback controller, and configures the dynamic UI layouts 
     * and signal-slot connections.
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

        // Channel Layout (Now dynamic)
        _channel_layout = new QVBoxLayout(); 
        _channel_title = new QLabel("Channel Selector:", this);
        _channel_layout->addWidget(_channel_title);

        _channel_group = new QButtonGroup(this);
        connect(_channel_group, QOverload<int>::of(&QButtonGroup::idClicked), this, &PianoUI::send_channel_selection);

        // Files Layout (Combobox is now for .mipi re-loading)
        auto* files_layout = new QVBoxLayout();
        _new_file_label = new QLabel("No File Selected", this);
        _new_file_button = new QPushButton("Select A MIDI File", this); 
        _old_file_button = new QComboBox(this);
        connect(_old_file_button, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PianoUI::load_existing_mipi);

        files_layout->addWidget(_new_file_label);
        files_layout->addWidget(_new_file_button);
        files_layout->addWidget(_old_file_button);

        // Speed Layout
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
        connect(_speed_control, &QSlider::valueChanged, this, &PianoUI::send_time_scale);
        connect(_track_slider, &QSlider::valueChanged, this, &PianoUI::update_status_info);

        main_layout->addStretch(2);
        
        // Setup existing MIPI files in the dropdown
        populate_mipi_combobox();
        update_status_info();
    }

    /**
     * @brief Destructor for the PianoUI class.
     */
    PianoUI::~PianoUI() {}

    // --- NEW HELPER FUNCTIONS ---

    /**
     * @brief Forces playback to pause and resets the track slider to zero.
     * * Updates the UI to reflect a paused state and sends an asynchronous ROS 2
     * request to halt the robot playback safely.
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
    }

    /**
     * @brief Scans the designated directory and populates the combo box with available .mipi files.
     * * Signals are temporarily blocked to prevent accidental load triggers while populating.
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
     * @brief Dynamically updates the channel selection radio buttons.
     * * Clears any existing radio buttons from the layout and generates new ones 
     * based on the instrument list parsed from the loaded file.
     */
    void PianoUI::update_channel_radio_buttons() {
        // Clear existing radio buttons
        QList<QAbstractButton*> buttons = _channel_group->buttons();
        for (QAbstractButton* btn : buttons) {
            _channel_group->removeButton(btn);
            _channel_layout->removeWidget(btn);
            btn->deleteLater();
        }

        // Add new radio buttons based on loaded data
        std::vector<std::string> instrument_list = processor.get_instrument_names();
        for (size_t i = 0; i < instrument_list.size(); ++i) {
            QRadioButton* rb = new QRadioButton(QString::fromStdString(instrument_list[i]), this);
            _channel_layout->addWidget(rb);
            _channel_group->addButton(rb, static_cast<int>(i)); 
        }
        
        // Auto-check the first one
        if (!instrument_list.empty()) {
            _channel_group->button(0)->setChecked(true);
        }
    }

    // --- STANDARD SLOTS ---

    /**
     * @brief Toggles the playback state between play and pause.
     * * Updates the play/pause button icon and sends the corresponding 
     * func service request over ROS 2.
     */
    void PianoUI::play_pause(){
        is_playing = !is_playing;
        _play_pause_button->setText(is_playing ? "▐▐" : "▶");
        
        auto request = std::make_shared<jamc::srv::Func::Request>();
        RCLCPP_INFO(this->get_logger(), is_playing ? "Sending PLAY request" : "Sending PAUSE request");

        if (!playback_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Playback service is not available.");
            return;
        }

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
     * @brief Opens a file dialog to process a new MIDI file.
     * * Converts a raw .mid file into a .mipi file format, updates the UI components,
     * and automatically triggers a load for the first valid channel.
     */
    void PianoUI::open_midi_file() {
        QString file_name = QFileDialog::getOpenFileName(this, "Select MIDI File", QDir::homePath(), "MIDI Files (*.mid *.midi)");        
        if (file_name.isEmpty()) return;

        std::string std_midi_path = file_name.toStdString();
        QString json_name = QFileInfo(file_name).baseName() + ".mipi";
        std::string std_json_name = json_name.toStdString();

        if (processor.processMidiFile(std_midi_path, std_json_name)) {
            _midi_file_path = QDir::homePath() + "/mipi_files/" + json_name;
            _new_file_label->setText(QFileInfo(file_name).fileName());

            update_channel_radio_buttons();
            populate_mipi_combobox(); 

            _track_slider->setRange(0, static_cast<int>(processor.get_song_duration()));
            update_status_info();
            
            if (!processor.get_channels().empty()) {
                send_channel_selection(0);
            }
            RCLCPP_INFO(this->get_logger(), "File processed successfully: %s", std_json_name.c_str());
        } else {
            _new_file_label->setText("Error: Could not process MIDI");
        }
    }

    /**
     * @brief Loads a previously converted .mipi file from the dropdown menu.
     * * @param index The integer index of the item selected in the combo box.
     */
    void PianoUI::load_existing_mipi(int index) {
        if (index <= 0) return; // Ignore the first prompt item

        QString mipi_filename = _old_file_button->itemText(index);
        
        if (processor.load_json_file(mipi_filename.toStdString())) {
            _midi_file_path = QDir::homePath() + "/mipi_files/" + mipi_filename;
            _new_file_label->setText(mipi_filename);
            
            update_channel_radio_buttons();
            
            _track_slider->setRange(0, static_cast<int>(processor.get_song_duration()));
            update_status_info();

            if (!processor.get_channels().empty()) {
                send_channel_selection(0);
            }
        } else {
            RCLCPP_ERROR(this->get_logger(), "Failed to load .mipi file.");
        }
    }

    /**
     * @brief Handles channel selection and sends a load request to the robot.
     * * Retrieves the actual MIDI channel mapped to the clicked button, forces a UI pause/reset,
     * and sends an asynchronous ROS 2 request with the file path and channel index.
     * * @param button_id The internal ID of the triggered radio button within the button group.
     */
    void PianoUI::send_channel_selection(int button_id) {
        if (button_id < 0 || _midi_file_path.isEmpty()) return;

        // Force stop and reset track when changing channel/instrument
        force_pause_and_reset();

        std::vector<int> channels = processor.get_channels();
        if (button_id >= static_cast<int>(channels.size())) return;
        int selected_channel = channels.at(button_id);

        auto request = std::make_shared<jamc::srv::Load::Request>();
        
        request->filepath = (QFileInfo(_midi_file_path).baseName() + ".mipi").toStdString();
        request->index = selected_channel; 

        RCLCPP_INFO(this->get_logger(), "Loading MIPI: %s on Channel: %d", 
                    request->filepath.c_str(), selected_channel);

        if (!channel_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Channel selection service not ready.");
            return;
        }

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
     * @brief Sends a ROS 2 request to set playback direction to forward.
     * * Ignored if the playback direction is already set to forward.
     */
    void PianoUI::set_direction_forward() {
        if (current_dir == PlaybackDirection::Forward) return;

        auto request = std::make_shared<jamc::srv::Func::Request>();
        RCLCPP_INFO(this->get_logger(), "Sending FORWARD direction request");
        
        if (!direction_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Direction service is not available.");
            return;
        }

        direction_client->async_send_request(request, 
            [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                try {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Direction response received: %s", response->message.c_str());
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Direction service failed: %s", e.what());
                }
            });
    }

    /**
     * @brief Sends a ROS 2 request to set playback direction to reverse.
     * * Ignored if the playback direction is already set to reverse.
     */
    void PianoUI::set_direction_reverse() {
        if (current_dir == PlaybackDirection::Reverse) return;

        auto request = std::make_shared<jamc::srv::Func::Request>();
        RCLCPP_INFO(this->get_logger(), "Sending REVERSE direction request");
        
        if (!direction_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "Direction service is not available.");
            return;
        }

        direction_client->async_send_request(request, 
            [this](rclcpp::Client<jamc::srv::Func>::SharedFuture future) {
                try {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "Direction response received: %s", response->message.c_str());
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Direction service failed: %s", e.what());
                }
            });
    }

    /**
     * @brief Updates visual UI strings detailing current system status.
     * * Refreshes the file status, playback speed percentage, and current track time against total duration.
     */
    void PianoUI::update_status_info() {
        _status_val->setText(_midi_file_path.isEmpty() ? "Status: No File" : "Status: Ready");
        
        int speed_percent = _speed_control->value();
        _speed_val->setText(QString("Speed: %1%").arg(speed_percent));

        _time_val->setText(QString("Time: %1 / %2")
            .arg(_track_slider->value())
            .arg(processor.get_song_duration()));
    }

    /**
     * @brief Reads the speed slider value and sends a TimeScale service request.
     * * Computes a double scale (0.01 to 1.0) and updates the status info display simultaneously.
     */
    void PianoUI::send_time_scale() {
        update_status_info();

        auto request = std::make_shared<jamc::srv::TimeScale::Request>();
        request->scale = static_cast<double>(_speed_control->value()) / 100.0;
        
        RCLCPP_INFO(this->get_logger(), "Sending TimeScale request: %f", request->scale);

        if (!time_scale_client->service_is_ready()) {
            RCLCPP_WARN(this->get_logger(), "TimeScale service is not available.");
            return;
        }

        time_scale_client->async_send_request(request,
            [this](rclcpp::Client<jamc::srv::TimeScale>::SharedFuture future) {
                try {
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "TimeScale updated successfully");
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "TimeScale service call failed: %s", e.what());
                }
            });
    }
}