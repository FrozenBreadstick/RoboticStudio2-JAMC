// test file for midi reading class

// includes
#include <iostream>
#include <vector>
#include <string>
#include "../../include/midi_processor/midi_processor.h"

// test functions
int test_notes();
int test_process();
int test_get_instrument_names();


int main() {
    
    std::cout << "Testing midi Processor class - v0.1" << std::endl;

    // Run test
    test_process();

    // close file
    return 0;
}


int test_process() {

    // create midi
    MidiProcessor midi;

    // twinkle twinkle little star
    // std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid";
    // mary had a little lamb
    std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/mary-had-a-little-lamb.mid";

    // open file
    if(!midi.processMidiFile(midi_file_path)) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    // declare variables
    std::vector<int> channels;
    std::vector<int> instruments;
    std::vector<std::vector<int>> notes;
    std::vector<std::vector<double>> durations;
    std::vector<std::vector<double>> timings;
    double duration;
    std::vector<std::vector<int>> assigned_keys;
    std::vector<std::vector<int>> keyboard_values;

    // get and print song duration
    duration = midi.get_song_duration();
    std::cout << "Song duration: " << duration << std::endl;


    std::cout << std::endl;


    // get and print channels
    channels = midi.get_channels();

    std::cout << "Channels: " << std::endl;
    for(size_t i = 0; i < channels.size(); i++) {
        std::cout << channels[i] << std::endl;
    }


    std::cout << std::endl;


    // get and print instruments
    instruments = midi.get_instruments();
    std::cout << "Instruments: " << std::endl;
    for(size_t i = 0; i < instruments.size(); i++) {
        std::cout << instruments[i] << std::endl;
    }


    std::cout << std::endl;


    // get and print notes, durations, timings (by channel)
    notes = midi.get_channel_notes();
    durations = midi.get_channel_note_durations();
    timings = midi.get_channel_note_timings();

    for(size_t i = 0; i < notes.size(); i++) {
        std::cout << "Notes for channel " << channels.at(i) << ": " << std::endl;
        std::cout << "Number of notes: " << notes.at(i).size() << std::endl;

        for(size_t j = 0; j < notes.at(i).size(); j++) {
            std::cout << notes.at(i).at(j) << " at time " << timings.at(i).at(j) << " with duration " << durations.at(i).at(j) << std::endl;
        }
    }


    std::cout << std::endl;


    // get and print keyboard values, assigned keys(by channel)
    keyboard_values = midi.get_keyboard_values();
    assigned_keys = midi.get_assigned_keys();
    
    for(size_t i = 0; i < keyboard_values.size(); i++) {
        std::cout << "keyboard values: " << std::endl;

        for(size_t j = 0; j < keyboard_values.at(i).size(); j++) {
            std::cout << keyboard_values.at(i).at(j) << std::endl;
        }

        std::cout << "Assigned keys: " << std::endl;
        for(size_t j = 0; j < assigned_keys.at(i).size(); j++) {
            std::cout << assigned_keys.at(i).at(j) << " at time " << timings.at(i).at(j) << " with duration " << durations.at(i).at(j) << std::endl;
        }

         std::cout << std::endl;
    }

    return 0;
}


int test_get_instrument_names() {

    // create midi
    MidiProcessor midi;

    // open file
    if(!midi.processMidiFile("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<std::string> instrument_names;

    instrument_names = midi.get_instrument_names();

    // display notes
    std::cout << "Instrument names: " << std::endl;
    for(size_t i = 0; i < instrument_names.size(); i++) {
        std::cout << instrument_names[i] << std::endl;
    }

    return 0;
}