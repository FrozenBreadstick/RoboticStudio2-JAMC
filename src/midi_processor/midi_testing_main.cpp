// test file for midi reading class

// includes
#include "midi_processor.h"
#include <iostream>
#include <vector>
#include <string>

// functions
int test_notes();
int test_instruments();


int main() {
    
    std::cout << "Testing midi Processor class - v0.1" << std::endl;

    // Run test
    test_instruments();

    // close file
    return 0;
}


int test_notes() {

    // create midi
    MidiProcessor midi;

    // open file
    if(!midi.open_file("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<std::string> notes;
    
    notes = midi.get_notes();

    // display notes
    std::cout << "Notes: " << std::endl;
    for(size_t i = 0; i < notes.size(); i++) {
        std::cout << notes[i] << std::endl;
    }

    return 0;
}

int test_instruments() {

    // create midi
    MidiProcessor midi;

    // open file
    if(!midi.open_file("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<std::vector<int>> instruments;
    
    instruments = midi.get_instruments();

    // display notes
    std::cout << "Channels - Instruments: " << std::endl;
    for(size_t i = 0; i < instruments.size(); i++) {

        std::cout << "Channel " << instruments[i][0] << " - Instrument " << instruments[i][1] << std::endl;
    }

    return 0;
}


