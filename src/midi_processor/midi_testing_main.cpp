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

    // open file
    if(!midi.processMidiFile("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<int> channels;
    std::vector<int> instruments;
    std::vector<std::vector<int>> notes;
    std::vector<std::vector<double>> durations;
    std::vector<std::vector<double>> timings;
    double duration;

    duration = midi.get_song_duration();
    std::cout << "Song duration: " << duration << std::endl;

    channels = midi.get_channels();

    std::cout << "Channels: " << std::endl;
    for(size_t i = 0; i < channels.size(); i++) {
        std::cout << channels[i] << std::endl;
    }

    instruments = midi.get_instruments();
    std::cout << "Instruments: " << std::endl;
    for(size_t i = 0; i < instruments.size(); i++) {
        std::cout << instruments[i] << std::endl;
    }

    notes = midi.get_channel_notes();
    durations = midi.get_channel_note_durations();
    timings = midi.get_channel_note_timings();

    for(size_t i = 0; i < notes.size(); i++) {
        std::cout << "Notes for channel " << channels.at(i) << ": " << std::endl;

        for(size_t j = 0; j < notes.at(i).size(); j++) {
            std::cout << notes.at(i).at(j) << " at time " << timings.at(i).at(j) << " with duration " << durations.at(i).at(j) << std::endl;
        }
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