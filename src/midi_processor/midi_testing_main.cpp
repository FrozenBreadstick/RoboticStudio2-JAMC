// test file for midi reading class

// includes
#include "midi_processor.h"
#include <iostream>
#include <vector>
#include <string>


int main() {
    std::cout << "Hello, world, from C++!" << std::endl;

    // create midi
    MidiProcessor midi;

    // open file
    midi.open_file("../../midi_files/twinkle-twinkle-little-star.mid");

    std::vector<std::string> notes;
    
    notes = midi.get_notes();

    std::cout << "Notes: " << std::endl;
    for(int i = 0; i < notes.size(); i++) {
        std::cout << notes[i] << std::endl;
    }

    // close file


    return 0;
}
