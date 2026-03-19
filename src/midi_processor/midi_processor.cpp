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
bool MidiProcessor::open_file(std::string midi_file_path) {

    std::cout << "Opening midi file: " << midi_file_path << std::endl;

    if(!midi.read(midi_file_path)) {
        std::cout << "Error opening midi file" << std::endl;
        return false;
    }
    
    midi.doTimeAnalysis();
    midi.linkNotePairs();

    std::cout << "Midi file read successfully" << std::endl;
    return true;

}

// get channels and correspeonding instruments --------------------------------
std::vector<std::vector<int>> MidiProcessor::get_instruments() {

    std::vector<std::vector<int>> instruments;

    // get instruments
    for(int i = 0; i < midi.getTrackCount(); i++) {
        for(int j = 0; j < midi.getEventCount(i); j++) {
            if(midi.getEvent(i, j).isTimbre()) {
                
                // pushback information
                int channel = midi.getEvent(i, j).getChannel();
                int instrument = midi.getEvent(i, j).getP1();

                std::vector<int> channel_instrument;
                channel_instrument.push_back(channel);
                channel_instrument.push_back(instrument);
                
                instruments.push_back(channel_instrument);
            }
        }
    }

    return instruments;
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