// includes -------------------------------------------------------------------
#include "midi_processor.h"

using namespace smf;

// constructor ----------------------------------------------------------------
MidiProcessor::MidiProcessor() {

    // create midi
    midi = MidiFile();

}


// destructor -----------------------------------------------------------------
MidiProcessor::~MidiProcessor() {

}


// functions ------------------------------------------------------------------

// open file
void MidiProcessor::open_file(std::string midi_file_path) {

    if(!midi.read(midi_file_path)) {
        std::cout << "Error opening midi file" << std::endl;
    }
    
    midi.doTimeAnalysis();
    midi.linkNotePairs();

    std::cout << "Midi file read successfully" << std::endl;

}

// get notes ------------------------------------------------------------------
std::vector<std::string> MidiProcessor::get_notes() {
    
    std::vector<std::string> notes;

    // get notes
    for(int i = 0; i < midi.getTrackCount(); i++) {
        for(int j = 0; j < midi.getEventCount(i); j++) {
            MidiEvent& event = midi.getEvent(i, j);
            if(event.isNoteOn()) {
                notes.push_back(std::to_string(event.getKeyNumber()));
            }
        }
    }

    return notes;
}