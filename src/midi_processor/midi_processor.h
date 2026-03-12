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

// libraries


// class
class MidiProcessor
{
    public:
        MidiProcessor();
        ~MidiProcessor();

    private:

        void open_file(std::string midi_file_path);

};


#endif