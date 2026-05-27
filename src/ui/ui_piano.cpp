/**
 * @file ui_piano.cpp
 * @brief Implementation of the PianoUI class for MIDI playback control.
 */

#include "ui/ui_piano.h"
#include <QList>
#include <chrono>

namespace UI
{
    /**
     * @brief Initializes the PianoUI Node and Qt widgets.
     * @details Sets up ROS 2 service clients, subscriptions, and the graphical layout.
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

        // Initialize ROS 2 clients
        playback_client = this->create_client<jamc::srv::Func>("/MIPI/play_pause");
        direction_client = this->create_client<jamc::srv::Func>("/MIPI/direction");
        time_scale_client = this->create_client<jamc::srv::TimeScale>("/MIPI/time_scale");
        channel_client = this->create_client<jamc::srv::Load>("/MIPI/load");
        debug_client = this->create_client<jamc::srv::Func>("/MIPI/debug");

        // Camera feed subscription
        _camera_sub = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/camera/color/image_raw", 
            10, 
            std::bind(&PianoUI::image_callback, this, std::placeholders::_1)
        );

        auto* main_layout = new QVBoxLayout(this);
        
        _title = new QLabel("MIDI To UR3", this);
        _title->setObjectName("MainTitle");
        _title->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(_title);
        main_layout->addStretch(1);

        // Camera view setup
        _camera_view = new QLabel("Waiting for camera feed...", this);
        _camera_view->setFixedSize(400, 300);
        _camera_view->setAlignment(Qt::AlignCenter);
        _camera_view->setStyleSheet("border: 2px solid #00aaff; background-color: black; border-radius: 5px; color: #00aaff;");
        main_layout->addWidget(_camera_view, 0, Qt::AlignCenter);

        // Watchdog timer to clear feed on disconnect
        _camera_frame_watch = new QTimer(this);
        _camera_frame_watch->setSingleShot(true);
        connect(_camera_frame_watch, &QTimer::timeout, this, [this]() {
            _camera_view->clear();
            _camera_view->setText("Waiting for camera feed...");
            _camera_view->setStyleSheet("border: 2px solid #00aaff; background-color: black; border-radius: 5px; color: #00aaff;");
        });

        // Playback timer with buffer for hardware lag
        _playback_timer = new QTimer(this);
        _playback_timer->setProperty("overtime_seconds", 0);
        _playback_timer->setProperty("overtime_limit", 15); 

        connect(_playback_timer, &QTimer::timeout, this, [this]() {
            if (!is_playing) return;
                
            int step = (current_dir == PlaybackDirection::Forward) ? 1 : -1;
            int new_time = _track_slider->value() + step;
                
            if (new_time >= 0 && new_time <= _track_slider->maximum()) {
                _track_slider->setValue(new_time);
                _playback_timer->setProperty("overtime_seconds", 0); 
            } else {
                if (current_dir == PlaybackDirection::Forward) {
                    int current_overtime = _playback_timer->property("overtime_seconds").toInt();
                    int max_overtime_allowed = _playback_timer->property("overtime_limit").toInt();

                    // Allow hardware to finish execution
                    if (current_overtime < max_overtime_allowed) {
                        _playback_timer->setProperty("overtime_seconds", current_overtime + 1);
                    } else {
                        force_pause_and_reset(); 
                        _playback_timer->setProperty("overtime_seconds", 0);
                    }
                } else {
                    force_pause_and_reset();
                }
            }
        });

        // Configuration layout
        auto* config_group = new QGroupBox("Configuration", this);
        auto* config_layout = new QHBoxLayout(config_group);

        auto* channel_section_layout = new QVBoxLayout(); 
        _channel_title = new QLabel("Channel Selector:", this);
        channel_section_layout->addWidget(_channel_title);

        _channel_layout = new QVBoxLayout(); 
        _channel_layout->setAlignment(Qt::AlignTop);

        auto* channel_container = new QWidget();
        channel_container->setLayout(_channel_layout);
        channel_container->setStyleSheet("background-color: transparent;"); 

        auto* channel_scroll = new QScrollArea(this);
        channel_scroll->setWidget(channel_container);
        channel_scroll->setWidgetResizable(true);
        channel_scroll->setFrameShape(QFrame::NoFrame);
        channel_scroll->setMinimumHeight(100);
        channel_scroll->setMaximumWidth(250); 
        
        channel_section_layout->addWidget(channel_scroll);

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

        config_layout->addLayout(channel_section_layout);
        config_layout->addLayout(files_layout);
        config_layout->addLayout(speed_layout);
        main_layout->addWidget(config_group);

        // Visualisation layout
        auto* roll_group = new QGroupBox("Sheet Music Visualisation", this);
        auto* roll_layout = new QVBoxLayout(roll_group);

        _roll_scene = new QGraphicsScene(this);
        _roll_view = new QGraphicsView(_roll_scene, this);
        _roll_view->setFixedHeight(150); 
        _roll_view->setStyleSheet("background-color: #121212; border: 1px solid #3a3a3a;"); 
        _roll_view->setRenderHint(QPainter::Antialiasing, false); 
        _roll_view->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);

        // Playhead setup
        _playhead = new QGraphicsLineItem();
        QPen playhead_pen(Qt::red);
        playhead_pen.setWidth(2);
        _playhead->setPen(playhead_pen);
        _playhead->setZValue(10); 
        _roll_scene->addItem(_playhead);

        roll_layout->addWidget(_roll_view);
        main_layout->addWidget(roll_group);

        // Playback controls
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

        // Status dashboard
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

        // Signal connections
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
     * @brief Destructor for PianoUI.
     */
    PianoUI::~PianoUI() {}

    /**
     * @brief Processes incoming ROS 2 image messages and renders them.
     * @param msg Shared pointer to the sensor_msgs::msg::Image.
     */
    void PianoUI::image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        static auto last_frame_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();

        // Throttle to 30 FPS
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_frame_time).count() < 33) {
            return;
        }

        last_frame_time = current_time;

        if (msg->encoding == "rgb8") {
            QImage open_img(&msg->data[0], msg->width, msg->height, msg->step, QImage::Format_RGB888);
            QImage clean_img = open_img.mirrored(true, false);
            QPixmap pixmap = QPixmap::fromImage(clean_img).scaled(_camera_view->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

            QMetaObject::invokeMethod(_camera_view, [this, pixmap]() {
                _camera_view->setPixmap(pixmap);
                _camera_frame_watch->start(1000);
            }, Qt::QueuedConnection);
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                "PianoUI Camera Warning: Expected 'rgb8' image encoding, but received '%s'", msg->encoding.c_str());
        }
    }

    /**
     * @brief Sends an execution request to the debug service.
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
     * @brief Halts playback tracking, pauses the robot, and resets the slider.
     */
    void PianoUI::force_pause_and_reset() {
        if (is_playing) {
            is_playing = false;
            _play_pause_button->setText("▶");
            _playback_timer->stop();
            
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
     * @brief Scans the ~/mipi_files directory and populates the cache dropdown list.
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
     * @brief Generates radio buttons corresponding to instrument channels in the active file.
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
     * @brief Toggles playback state and transmits the command to the hardware node.
     */
    void PianoUI::play_pause() {
        is_playing = !is_playing;
        _play_pause_button->setText(is_playing ? "▐▐" : "▶");

        if (is_playing) {
            double scale = static_cast<double>(_speed_control->value()) / 100.0;
            _playback_timer->start(1000 / scale); 
        } else {
            _playback_timer->stop();
        }
        
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
     * @brief Opens a file dialog to select and parse a raw MIDI file.
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
     * @brief Loads a cached .mipi file directly from the dropdown menu.
     * @param index Combobox selection index.
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
     * @brief Transmits target instrument channel load instructions to the robot controller.
     * @param button_id Associated radio button identifier.
     */
    void PianoUI::send_channel_selection(int button_id) {
        if (button_id < 0 || _midi_file_path.isEmpty()) return;

        force_pause_and_reset();
        draw_piano_roll(button_id);

        std::vector<std::vector<int>> all_notes = processor.get_channel_notes();
        if (button_id >= 0 && button_id < static_cast<int>(all_notes.size())) {
            size_t note_count = all_notes[button_id].size();

            // Scale padding dynamically based on note density
            int calculated_padding = std::max(10, static_cast<int>(note_count * 0.25));
            
            if (_playback_timer) {
                _playback_timer->setProperty("overtime_limit", calculated_padding);
            }
        }

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
     * @brief Alternates standard playback navigation sequence logic.
     */
    void PianoUI::toggle_direction() {
        if (current_dir == PlaybackDirection::Forward) set_direction_reverse();
        else set_direction_forward();
    }

    /**
     * @brief Triggers direct forward trajectory execution mode.
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
     * @brief Triggers direct inverse trajectory execution mode.
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
     * @brief Updates system label properties mapping to live state variants.
     */
    void PianoUI::update_status_info() {
        _status_val->setText(_midi_file_path.isEmpty() ? "Status: No File" : "Status: Ready");
        int speed_percent = _speed_control->value();
        _speed_val->setText(QString("Speed: %1%").arg(speed_percent));
        _time_val->setText(QString("Time: %1 / %2").arg(_track_slider->value()).arg(processor.get_song_duration()));

        if (_playhead && _roll_scene) { 
            const double time_scale = 10.0; 
            double current_x = _track_slider->value() * time_scale;

            double scene_height = _roll_scene->height();
            _playhead->setLine(current_x, 0, current_x, scene_height > 0 ? scene_height : 150);

            _roll_view->centerOn(current_x, _roll_scene->height() / 2);
        }
    }

    /**
     * @brief Propagates time scale adjustments to execution systems.
     */
    void PianoUI::send_time_scale() {
        update_status_info();
        auto request = std::make_shared<jamc::srv::TimeScale::Request>();
        request->scale = static_cast<double>(_speed_control->value()) / 100.0;

        if (_playback_timer->isActive()) {
            _playback_timer->setInterval(1000 / request->scale);
        }
        
        if (!time_scale_client->service_is_ready()) return;
        time_scale_client->async_send_request(request);
    }

    /**
     * @brief Generates graphic layout block representation matching parsed note timings.
     * @param channel_index Data channel list identifier.
     */
    void PianoUI::draw_piano_roll(int channel_index) {
        _roll_scene->removeItem(_playhead);
        _roll_scene->clear();
        _roll_scene->addItem(_playhead);

        std::vector<std::vector<int>> all_notes = processor.get_channel_notes();
        std::vector<std::vector<double>> all_timings = processor.get_channel_note_timings();
        std::vector<std::vector<double>> all_durations = processor.get_channel_note_durations();

        if (channel_index < 0 || channel_index >= static_cast<int>(all_notes.size()) || all_notes[channel_index].empty()) {
            return; 
        }

        std::vector<int> channel_notes = all_notes[channel_index];
        std::vector<double> channel_timings = all_timings[channel_index];
        std::vector<double> channel_durations = all_durations[channel_index];

        int max_pitch = -1;
        int min_pitch = 999;
        for (int p : channel_notes) {
            if (p > max_pitch) max_pitch = p;
            if (p < min_pitch) min_pitch = p;
        }

        const double time_scale = 10.0; 
        const double pitch_scale = 4.0; 

        double note_block_height = (max_pitch - min_pitch + 1) * pitch_scale;
        
        double view_height = 150.0; 
        double y_offset = (view_height - note_block_height) / 2.0;

        for (size_t i = 0; i < channel_notes.size(); ++i) {
            
            double start_time = channel_timings[i];
            double duration = channel_durations[i];
            int pitch = channel_notes[i]; 

            double x = start_time * time_scale;
            double y = y_offset + (max_pitch - pitch) * pitch_scale; 
            
            double width = std::max(duration * time_scale, 2.0); 
            double height = pitch_scale;

            QGraphicsRectItem* rect = _roll_scene->addRect(x, y, width, height);
            rect->setBrush(QBrush(QColor("#00aaff"))); 
            rect->setPen(QPen(Qt::NoPen)); 
        }

        _roll_scene->setSceneRect(_roll_scene->itemsBoundingRect());
    }
}