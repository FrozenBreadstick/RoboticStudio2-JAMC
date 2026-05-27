
#include "audio/audio.h"

/**
 * @brief Standalone Class for detecting and processing microphone audio. Detects audio from a piano and processes it to determine which note it is then publishes that to a topic. 
 * 
 * @details This class contains the following ROS2 I/O
 * - **Subscribers**
 *  - '-/audio/audio' (audio_msgs/msg/AudioData): Receives audio data from the microphone
 * - **Publishers**
 *  - '-/audio/notes' (audio_msgs/msg/Notes): Publishes detected notes to a topic
 */

// Constructor
Audio::Audio(): Node("audio")
{
    RCLCPP_INFO(this->get_logger(), "Audio node has been started. Starting initilisation movement");
    audio_sub_ = this->create_subscription<audio_msgs::msg::AudioData>("/audio/audio", 10, std::bind(&Audio::audio_callback, this, std::placeholders::_1));
    notes_pub_ = this->create_publisher<audio_msgs::msg::Notes>("/audio/notes", 10);
    //debug_pub_ = this->create_publisher<std_msgs::msg::String>("/debug", 10);
}


// Destructor
Audio::~Audio()
{
    RCLCPP_INFO(this->get_logger(), "Audio node is shutting down.");
}

/**
 * @brief Callback function for the audio subscriber, used to receive audio data from the microphone
 * @param msg The audio data received from the microphone
 */
void Audio::audio_callback(const audio_msgs::msg::AudioData::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "Received audio data from microphone");
    process_audio(msg);
}

/**
 * @brief Callback function for the notes publisher, used to publish detected notes to a topic
 * @param msg The detected notes to be published
 */
void Audio::notes_callback(const audio_msgs::msg::Notes::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "Received notes from audio detection");
    notes_pub_->publish(msg);
}