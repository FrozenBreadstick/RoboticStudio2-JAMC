// include guards

#ifndef MIDI_PROCESSOR_H
#define MIDI_PROCESSOR_H

// includes

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

// MidiFile library
#include "midiLibrary/include/Binasc.h"
#include "midiLibrary/include/MidiFile.h"
#include "midiLibrary/include/MidiEvent.h"
#include "midiLibrary/include/MidiMessage.h"
#include "midiLibrary/include/MidiEventList.h"
#include "midiLibrary/include/Options.h"

using namespace smf;

// class
class MidiProcessor
{
    public:
        MidiProcessor();
        ~MidiProcessor();

        void open_file(std::string midi_file_path);

        std::vector<std::string> get_notes();

    private:

        // variables
        MidiFile midi;
        

};


#endif