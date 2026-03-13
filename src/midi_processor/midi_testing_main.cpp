// test file for midi reading class

// includes
#include "midi_processor.h"
#include <iostream>
#include <vector>
#include <string>


int main() {
    std::cout << "Testing midi Processor class - v0.1" << std::endl;

    // create midi
    MidiProcessor midi;

    // open file
    if(!midi.open_file("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<std::string> notes;
    
    notes = midi.get_notes();

    std::cout << "Notes: " << std::endl;
    for(size_t i = 0; i < notes.size(); i++) {
        std::cout << notes[i] << std::endl;
    }

    // close file


    return 0;
}
