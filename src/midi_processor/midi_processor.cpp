// includes -------------------------------------------------------------------
    #include "../../include/midi_processor/midi_processor.h"

// Namespace
    using namespace smf;

// Core functions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
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


// primary public functions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                if(!process_channel_notes_with_timings(channels.at(i))) {
                    return false;
                }
            }

            // filter chords
            if(!filter_chords()) {
                return false;
            }

            // process song duration
            if(!process_song_duration()) {
                return false;
            }

            // save data
            if(!save_midi_data()) {
                return false;
            }

            return true;
        }

    // get channels --------------------------------------------------------------
        std::vector<int> MidiProcessor::get_channels()
        {
            return channels;
        }

    // get instruments ----------------------------------------------------------
        std::vector<int> MidiProcessor::get_instruments()
        {
            return instruments;
        }

    // get instrument names and channels as a string vector ----------------------
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

    // gets all notes that correspond to a specific channel ----------------------
        std::vector<std::vector<int>> MidiProcessor::get_channel_notes() 
        {
            std::cout << "Getting channel notes" << std::endl;
            std::cout << "notes size: " << notes.size() << std::endl;
            std::cout << "notes at 0 size: " << notes.at(0).size() << std::endl;
            std::cout << "notes vector" << notes.at(0).size() << std::endl;
            return notes;
        }

    // gets all note timings that correspond to a specific channel ---------------
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_timings() 
        {
            std::cout << "Getting channel note timings" << std::endl;
            return note_timeStamps;
        }

    // gets all note durations that correspond to a specific channel -------------
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_durations() 
        {
            std::cout << "Getting channel note durations" << std::endl;
            return note_durations;
        }

    // get song duration ---------------------------------------------------------
        double MidiProcessor::get_song_duration() 
        {
            return fileDuration;
        }

    // load JSON file -----------------------------------------------------------
        bool MidiProcessor::load_json_file(std::string json_file_path) 
        {
            std::cout << "Loading JSON file: " << json_file_path << std::endl;

            return true;
        }


// primary private functions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // open file ------------------------------------------------------------------
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

                        for(unsigned int i; i < channels.size(); i++) {

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


    // process channel notes WITH timings ------------------------------------------
        bool MidiProcessor::process_channel_notes_with_timings(int channel)
        {

            std::vector<int> channel_notes;
            std::vector<double> channel_note_durations;
            std::vector<double> note_on_timeStamps;



            // get notes for specified channel
            for(int i = 0; i < midi.getTrackCount(); i++) {
                for(int j = 0; j < midi.getEventCount(i); j++) {
                    MidiEvent& event = midi.getEvent(i, j);
                    if(event.isNoteOn()) {
                        if(event.getChannel() == channel) {
                            channel_notes.push_back(event.getKeyNumber());
                            channel_note_durations.push_back(event.getDurationInSeconds());
                            note_on_timeStamps.push_back(midi.getTimeInSeconds(i, j));
                        }
                    }
                }
            }

            if(channel_notes.size() == 0) {
                std::cout << "No notes found" << std::endl;
                return false;
            }

            notes.push_back(channel_notes);
            note_timeStamps.push_back(note_on_timeStamps);
            note_durations.push_back(channel_note_durations);

            return true;
        }

    // filters chords down to only the root note ----------------------------------
        bool MidiProcessor::filter_chords()
        {
            // for each set of notes
            for(size_t i = 0; i < notes.size(); i++) {

                // get timestamps for first note of current channel
                double current_stamp = note_timeStamps.at(i).back();
                std::vector<int> current_indexs;


                // for each timestamp
                int j = note_timeStamps.at(i).size() - 1;
                bool new_note = true;

                while(j >= 0) {

                    std::cout << "j: " << j << " note: " << notes.at(i).at(j) << " time stamp: " << note_timeStamps.at(i).at(j) << std::endl;

                    // get timestamp
                    double note_timeStamp = note_timeStamps.at(i).at(j);

                    // check if note_timestamp is new 
                    if(note_timeStamp == current_stamp) {
                        current_indexs.push_back(j);
                        j--;
                        new_note = false;
                    }
                    
                    if(j == -1 || new_note == true) {

                        if(current_indexs.size() != 0 && current_indexs.size() != 1) {

                            bool high_note_filter;
                            int kept_note_pitch;
                            int kept_note_index;

                            if(instruments.at(i) <= 23) {
                                // apply high note filter
                                high_note_filter = true;
                                kept_note_pitch = -1;
                            }
                            else {
                                // apply low note filter
                                high_note_filter = false;
                                kept_note_pitch = 128;
                            }


                            // for each index in current_indexs
                            for(size_t k = 0; k < current_indexs.size(); k++) {

                                int note_pitch = notes.at(i).at(current_indexs.at(k));

                                // apply filter to keep only 1 note
                                if(high_note_filter){
                                    if(note_pitch > kept_note_pitch) {
                                        kept_note_pitch = note_pitch;
                                        kept_note_index = k;
                                    }
                                }
                                else {
                                    if(note_pitch < kept_note_pitch) {
                                        kept_note_pitch = note_pitch;
                                        kept_note_index = k;
                                    }
                                }

                            }
                            std::cout << " " << std::endl;
                            std::cout << "pre index erase: " << current_indexs.size() << std::endl;
                            // for(size_t k = 0; k < current_indexs.size(); k++) {
                            //     std::cout << "current_indexs: " << current_indexs.at(k) << std::endl;
                            // }

                            current_indexs.erase(current_indexs.begin() + kept_note_index);
                            
                            std::cout << "post index erase: " << current_indexs.size() << std::endl;
                            // for(size_t k = 0; k < current_indexs.size(); k++) {
                            //     std::cout << "current_indexs: " << current_indexs.at(k) << std::endl;
                            // }                        

                            //std::sort(current_indexs.begin(), current_indexs.end(), std::greater<int>());
                            std::cout << " " << std::endl;
                            
                            for(size_t k = 0; k < current_indexs.size(); k++) {

                                int index = current_indexs.at(k);

                                std::cout << "index: " << index << std::endl;

                                std::cout << "pre erase - notes size: " << notes.at(i).size() << " notes duration size: " << note_durations.at(i).size() << " notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;

                                if(notes.at(i).size() != note_durations.at(i).size() || notes.at(i).size() != note_timeStamps.at(i).size()) {
                                    std::cout << "Error erasing notes" << std::endl;
                                    std::cout << "notes size: " << notes.at(i).size() << " notes duration size: " << note_durations.at(i).size() << " notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;
                                }

                                notes.at(i).erase(notes.at(i).begin() + index);
                                note_durations.at(i).erase(note_durations.at(i).begin() + index);
                                note_timeStamps.at(i).erase(note_timeStamps.at(i).begin() + index);

                                std::cout << "post erase - notes size: " << notes.at(i).size() << " notes duration size: " << note_durations.at(i).size() << "notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;

                                if(notes.at(i).size() != note_durations.at(i).size() || notes.at(i).size() != note_timeStamps.at(i).size()) {
                                    std::cout << "Error erasing notes" << std::endl;
                                    std::cout << "notes size: " << notes.at(i).size() << "notes duration size: " << note_durations.at(i).size() << "notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;
                                }

                                std::cout << " " << std::endl;
                            }

                            current_indexs.clear();

                        }
                        else {
                            std::cout << "ignoring note (only 1 note)" << std::endl;
                            current_indexs.clear();
                        }
                    }

                    // reset the current stamp
                    new_note = true;
                    current_stamp = note_timeStamp;
                }

            }


            return true;
        }


    // process song duration ------------------------------------------------------
        bool MidiProcessor::process_song_duration()
        {
            double duration;

            // sort tracks
            midi.sortTracksNoteOnsBeforeOffs();

            // get song duration
            duration = midi.getFileDurationInSeconds();

            fileDuration = duration;

            return true;
        }

    // stores all data for current midi file in a storage file --------------------
        bool MidiProcessor::save_midi_data()
        {
            return true;
        }