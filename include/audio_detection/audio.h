#ifndef AUDIO_H
#define AUDIO_H

#include <rclcpp/rclcpp.hpp>
#include <audio_msgs/msg/audio_data.hpp>
#include <audio_msgs/msg/notes.hpp>
#include <std_msgs/msg/string.hpp>

namespace Audio
{
    class Audio : public rclcpp::Node
    {
        public:
            Audio();
            ~Audio();

        private:
            rclcpp::Subscription<audio_msgs::msg::AudioData>::SharedPtr audio_sub_;
            rclcpp::Publisher<audio_msgs::msg::Notes>::SharedPtr notes_pub_;
            rclcpp::Publisher<std_msgs::msg::String>::SharedPtr debug_pub_;

            // TODO: Add a subscriber callback for the audio topic
            void audio_callback(const audio_msgs::msg::AudioData::SharedPtr msg);

            // TODO: Add a publisher callback for the notes topic
            void notes_callback(const audio_msgs::msg::Notes::SharedPtr msg);

            // TODO: Add a process that measures the distance between peaks of the audio signal, calcs period, converts to frequency, then calcs an approximate note based on the frequency.
            /*
             -  Measure the distance between 2 positive zero crossings in samples,
                either by counting samples in a buffer, or by triggering a
                counter/timer to start/stop with a comparator (analog or otherwise).
                The distance between zero crossings could be an estimate of the
                period of a sine wave if there isn't much noise in the signal
                and the count rate is high enough relative to the frequency
                of the signal.  The frequency will be the reciprocal of the period
                distance or time, and the accuracy will be limited by the sample
                spacing. */
            void process_audio(const audio_msgs::msg::AudioData::SharedPtr msg);
    
        // variables
            // TODO: Add a variable to store the frequency of the detected note
            double frequency;
            // TODO: Add a variable to store the period of the detected note
            double period;
            // TODO: Add a variable to store the approximate note of the detected note
            int note;

            // TODO: Add a thread and a mutex to process the audio data in a separate thread
            boost::thread audio_thread;
            boost::mutex audio_mutex;
    
    };
}