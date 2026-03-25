// includes -------------------------------------------------------------------
    #include "../../include/midi_processor/midi_processor.h"

// Namespace
    using namespace smf;

// Core functions --------------------------------------------------------------
    
    // constructor ----------------------------------------------------------------
        MidiProcessor::MidiProcessor() 
        {
            // create midi
            midi = MidiFile();

        }


    // destructor -----------------------------------------------------------------
        MidiProcessor::~MidiProcessor() 
        {

        }


// primary public functions ----------------------------------------------------

    // processes a midi file by saving all instruments (and their channels) to a vector and all notes belonging to a channel to a vector
        bool MidiProcessor::processMidiFile(std::string midi_file_path)
        {
            // open file
            if(!open_file(midi_file_path)) {
                return false;
            }

            // process instruments and channels
            if(!process_instruments()) {
                return false;
            }

            // get notes
            for(size_t i = 0; i < channels.size(); i++) {
                if(!process_channel_notes(channels.at(i))) {
                    return false;
                }
            }

            // save data
            if(!save_midi_data()) {
                return false;
            }

            return true;
        }

    // get channels
        std::vector<int> MidiProcessor::get_channels()
        {
            return channels;
        }

    // get instruments
        std::vector<int> MidiProcessor::get_instruments()
        {
            return instruments;
        }

    // get instrument names and channels as a string vector
        std::vector<std::string> MidiProcessor::get_instrument_names()
        {
            std::vector<std::string> instrument_names;

            for(size_t i = 0; i < instruments.size(); i++) {
                
                std::string name = midi.getGMInstrumentName(instruments.at(i));
                std::string channel = std::to_string(channels.at(i));
                
                instrument_names.push_back(channel + " - " + name);

            }

            return instrument_names;
        }

    // gets all notes that correspond to a specific channel
        std::vector<std::vector<int>> MidiProcessor::get_channel_notes() 
        {
            return notes;
        }


// primary private functions ---------------------------------------------------

    // open file
        bool MidiProcessor::open_file(std::string midi_file_path) 
        {

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

    // process instruments and channels -------------------------------------------
        bool MidiProcessor::process_instruments() 
        {

            // check events
            for(int i = 0; i < midi.getTrackCount(); i++) {
                for(int j = 0; j < midi.getEventCount(i); j++) {
                    if(midi.getEvent(i, j).isTimbre()) {

                        // get channel and instrument
                        int channel = midi.getEvent(i, j).getChannel();
                        int instrument = midi.getEvent(i, j).getP1();
                        
                        // check for dupe channels
                        bool dupe = false;

                        for(int i; i < channels.size(); i++) {

                            if(channels.at(i) == channel && instruments.at(i) == instrument) {
                                dupe = true;
                            } 
                        }

                        // pushback information
                        if(dupe == false) {
                            channels.push_back(channel);
                            instruments.push_back(instrument);
                        }
                    }
                }
            }

            if(channels.size() == 0) {
                std::cout << "No channels found" << std::endl;
                return false;
            }

            return true;
        }


    // process channel notes -------------------------------------------------------
        bool MidiProcessor::process_channel_notes(int channel) 
        {

            std::vector<int> channel_notes;

            // get notes for specified channel
            for(int i = 0; i < midi.getTrackCount(); i++) {
                for(int j = 0; j < midi.getEventCount(i); j++) {
                    MidiEvent& event = midi.getEvent(i, j);
                    if(event.isNoteOn()) {
                        if(event.getChannel() == channel) {
                            channel_notes.push_back(event.getKeyNumber());
                        }
                    }
                }
            }

            if(channel_notes.size() == 0) {
                std::cout << "No notes found" << std::endl;
                return false;
            }

            notes.push_back(channel_notes);

            return true;
        }


    // stores all data for current midi file in a storage file
        bool MidiProcessor::save_midi_data()
        {
            return true;
        }



// bonus functions --------------------------------------------------------------

    // get notes ------------------------------------------------------------------
        std::vector<int> MidiProcessor::get_notes() 
        {
            
            std::vector<int> notes;

            // get notes
            for(int i = 0; i < midi.getTrackCount(); i++) {
                for(int j = 0; j < midi.getEventCount(i); j++) {
                    MidiEvent& event = midi.getEvent(i, j);
                    if(event.isNoteOn()) {
                        notes.push_back(event.getKeyNumber());
                    }
                }
            }

            return notes;
        }