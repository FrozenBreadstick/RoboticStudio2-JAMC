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
        int MidiProcessor::processMidiFile(std::string midi_file_path, std::string json_file_name)
        {
            // clear all variables
            channels.clear();
            instruments.clear();
            notes.clear();
            note_timeStamps.clear();
            note_durations.clear();
            fileDuration = 0;
            assigned_keys.clear();
            keyboard_values.clear();

            // open file
            if(!open_file(midi_file_path)) {
                return 1;
            }



            // process instruments and channels
            if(!process_instruments()) {
                return 2;
            }

            // get notes
            for(size_t i = 0; i < channels.size(); i++) {
                if(!process_channel_notes_with_timings(channels.at(i))) {
                    return 3;
                }
            }

            // filter chords
            if(!filter_chords()) {
                return 4;
            }

            // trim note durations
            if(!trim_note_durations()) {
                return 5;
            }

            // process song duration
            if(!process_song_duration()) {
                return 6;
            }

            // assign keys
            if(!assign_keys()) {
                return 7;
            }

            //std::string file_name = "test_name.mipi";

            // save data
            if(!save_midi_data(json_file_name)) {
                return 8;
            }

            // print data
            if(!debug_print_data()) {
                return 9;
            }

            return 0;
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
            return notes;
        }

    // gets all note timings that correspond to a specific channel ---------------
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_timings() 
        {
            return note_timeStamps;
        }

    // gets all note durations that correspond to a specific channel -------------
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_durations() 
        {
            return note_durations;
        }

    // get song duration ---------------------------------------------------------
        double MidiProcessor::get_song_duration() 
        {
            return fileDuration;
        }

    // get assigned keys ---------------------------------------------------------
        std::vector<std::vector<int>> MidiProcessor::get_assigned_keys() 
        {
            return assigned_keys;
        }

    // get keyboard values ---------------------------------------------------------
        std::vector<std::vector<int>> MidiProcessor::get_keyboard_values() 
        {
            return keyboard_values;
        }

    // get keyboard indexs ---------------------------------------------------------
        std::vector<std::vector<int>> MidiProcessor::get_keyboard_indexs() 
        {
            return keyboard_indexs;
        }

    // load JSON file -----------------------------------------------------------
        bool MidiProcessor::load_json_file(std::string json_file_path) 
        {
            // file path for loading
            std::filesystem::path file_path = std::filesystem::path(homeDir) / "mipi_files" / json_file_path;

            std::cout << "Loading JSON file: " << file_path << std::endl;

            // create json object
            json j;

            // open file
            std::ifstream file;
            file.open(file_path);

            // read file
            j = json::parse(file);

            // close file
            file.close();

            // get data from JSON object
            channels = j["channels"].get<std::vector<int>>();
            instruments = j["instruments"].get<std::vector<int>>();
            notes = j["notes"].get<std::vector<std::vector<int>>>();
            note_timeStamps = j["note_timeStamps"].get<std::vector<std::vector<double>>>();
            note_durations = j["note_durations"].get<std::vector<std::vector<double>>>();
            fileDuration = j["song_duration"].get<double>();
            assigned_keys = j["assigned_keys"].get<std::vector<std::vector<int>>>();
            keyboard_values = j["keyboard_values"].get<std::vector<std::vector<int>>>();
            keyboard_indexs = j["keyboard_indexs"].get<std::vector<std::vector<int>>>();

            std::cout << "Loaded midi data" << std::endl;

            // clear json object for next file
            j.clear();

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

                            // remove safe note from list of indexs to kill
                            current_indexs.erase(current_indexs.begin() + kept_note_index);
                            
                            // remove all other notes from the arrays
                            for(size_t k = 0; k < current_indexs.size(); k++) {

                                int index = current_indexs.at(k);

                                notes.at(i).erase(notes.at(i).begin() + index);
                                note_durations.at(i).erase(note_durations.at(i).begin() + index);
                                note_timeStamps.at(i).erase(note_timeStamps.at(i).begin() + index);

                                // error checking
                                if(notes.at(i).size() != note_durations.at(i).size() || notes.at(i).size() != note_timeStamps.at(i).size()) {
                                    std::cout << "Error erasing notes" << std::endl;
                                    std::cout << "notes size: " << notes.at(i).size() << "notes duration size: " << note_durations.at(i).size() << "notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;
                                }
                            }
                        }
                        
                        // reset the list of indexs
                        current_indexs.clear();
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

    
    // key assignment function ------------------------------------------------------
        bool MidiProcessor::assign_keys()
        {

            // keyboard min max values (around middle C)
            int keyboard_min = 41;
            int keyboard_max = 77;

            if(channels.size() == 0) {
                std::cout << "No channels found" << std::endl;
                return false;
            }

            // for each channel
            for(size_t i = 0; i < channels.size(); i++) {

                // current list of keys
                std::vector<int> keys;

                // current keyboard values
                std::vector<int> this_keyboard_values;

                // finding average key value per channel
                int average_key = 0;
             
                for(size_t j = 0; j < notes.at(i).size(); j++) {

                    average_key += notes.at(i).at(j);

                }

                average_key = average_key / notes.at(i).size();

                // get key name
                int  key_name = average_key % 12;

                // find diff between middle C and average key
                int diff = 60 + key_name - average_key;

                if(diff > 36) {
                    diff = 36;
                }
                else if(diff < -48) {
                    diff = -48;
                }

                int this_keyboard_max = keyboard_max - diff;
                int this_keyboard_min = keyboard_min - diff;

                // set keyboard values for the channel
                for(int k = 0; k < 37; k++) {
                    this_keyboard_values.push_back(this_keyboard_min + k);
                }

                keyboard_values.push_back(this_keyboard_values);

                // iteratite through all notes pushing back if within bounds and rounding to nearest note in bounds if not
                for(size_t j = 0; j < notes.at(i).size(); j++) {
                    
                    int note = notes.at(i).at(j);

                    if(note >= this_keyboard_min && note <= this_keyboard_max) {
                        keys.push_back(note);
                    }
                    else if(note < this_keyboard_min) {

                        while(note < this_keyboard_min) {
                            note += 12;
                        }

                        keys.push_back(note);
                    }
                    else if(note > this_keyboard_max) {

                        while(note > this_keyboard_max) {
                            note -= 12;
                        }

                        keys.push_back(note);
                    }

                }

                // pushback keys for this channel
                assigned_keys.push_back(keys);

                // process the assigned keys and find their indexs in the keyboard values vector and save them in a new keyboard_indexs vector
                std::vector<int> indexs;

                for(size_t j = 0; j < assigned_keys.at(i).size(); j++) {

                    // default index to 0
                    int index = 0;

                    for(size_t k = 0; k < keyboard_values.at(i).size(); k++) {
                        if(assigned_keys.at(i).at(j) == keyboard_values.at(i).at(k)) {
                            index = k;
                            break;
                        }
                    }

                    indexs.push_back(index);
                }

                // store the keyboard indexs in the keyboard_indexs vector
                keyboard_indexs.push_back(indexs);

            }

            return true;
        }


    // note duration trimming function ------------------------------------------------------
        bool MidiProcessor::trim_note_durations()
        {
            // for each set of notes
            for(size_t i = 0; i < notes.size(); i++) {

                for(size_t j = 0; j < notes.at(i).size(); j++) {

                    // get timestamp
                    double this_note_timeStamp = note_timeStamps.at(i).at(j);

                    // get duration
                    double this_note_duration = note_durations.at(i).at(j);

                    // get next timestamp
                    if(j + 1 >= notes.at(i).size()) {
                        break;
                    }
                    double next_note_timeStamp = note_timeStamps.at(i).at(j + 1);

                    // check diff between timestamps
                    double time_diff = next_note_timeStamp - this_note_timeStamp;

                    // check if note_duration is longer than next_note_duration
                    if(this_note_duration > time_diff) {

                        // trim note_duration
                        this_note_duration = time_diff - min_note_duration;

                        note_durations.at(i).at(j) = this_note_duration;
                    }
                }
            }
        }



    // stores all data for current midi file in a storage file --------------------
        bool MidiProcessor::save_midi_data(std::string file_name)
        {

            // create json object
            json j;

            // store data into JSON object
            std::cout << "Saving midi data" << std::endl;

            j["channels"] = channels;
            j["instruments"] = instruments;
            j["notes"] = notes;
            j["note_timeStamps"] = note_timeStamps;
            j["note_durations"] = note_durations;
            j["song_duration"] = fileDuration;
            j["assigned_keys"] = assigned_keys;
            j["keyboard_values"] = keyboard_values;
            j["keyboard_indexs"] = keyboard_indexs;

            // create file path
            std::filesystem::path folder = std::filesystem::path(homeDir) / "mipi_files";

            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
                std::cout << "Folder created!\n";
            } else {
                std::cout << "Folder already exists.\n";
            }

            std::filesystem::path file_path = folder / file_name;

            // open file
            std::ofstream file(file_path);
            if(!file.is_open()) {
                std::cout << "Error opening file!\n" << std::endl;
                return false;
            }

            // dump to file
            file << j;

            // close file
            file.close();

            // clear json object for next file
            j.clear();

            return true;
        }


        /*! @brief prints all data to the console
         *
         *  @return bool - returns true if the data was successfully printed
         * 
         *  @details This function is called wherever the user wants to test the data. It prints all stored the data to the console.
         */
        bool MidiProcessor::debug_print_data()
        {

            // get and print song duration
            std::cout << "Song duration: " << fileDuration << std::endl;


            std::cout << std::endl;

            // get and print channels
            std::cout << "Channels: " << std::endl;
            for(size_t i = 0; i < channels.size(); i++) {
                std::cout << channels.at(i) << std::endl;
            }


            std::cout << std::endl;


            // get and print instruments
            std::cout << "Instruments: " << std::endl;
            for(size_t i = 0; i < instruments.size(); i++) {
                std::cout << instruments.at(i) << std::endl;
            }


            std::cout << std::endl;


            // get and print notes, durations, timings (by channel)
            for(size_t i = 0; i < notes.size(); i++) {
                std::cout << "Notes for channel " << channels.at(i) << ": " << std::endl;
                std::cout << "Number of notes: " << notes.at(i).size() << std::endl;

                for(size_t j = 0; j < notes.at(i).size(); j++) {
                    std::cout << notes.at(i).at(j) << " at time " << note_timeStamps.at(i).at(j) << " with duration " << note_durations.at(i).at(j) << std::endl;
                }

                std::cout << std::endl;
            }


            std::cout << std::endl;


            // get and print keyboard values, assigned keys(by channel)
            for(size_t i = 0; i < keyboard_values.size(); i++) {
                std::cout << "keyboard values: " << std::endl;

                for(size_t j = 0; j < keyboard_values.at(i).size(); j++) {
                    std::cout << keyboard_values.at(i).at(j) << std::endl;
                }

                std::cout << std::endl;

                std::cout << "Assigned keys: " << std::endl;
                for(size_t j = 0; j < assigned_keys.at(i).size(); j++) {
                    std::cout << assigned_keys.at(i).at(j) << " at time " << note_timeStamps.at(i).at(j) << " with duration " << note_durations.at(i).at(j) << std::endl;
                }

                std::cout << std::endl;
            }

            std::cout << std::endl;

            // get and print keyboard indexs
            for(size_t i = 0; i < keyboard_indexs.size(); i++) {
                std::cout << "keyboard indexs: " << std::endl;

                for(size_t j = 0; j < keyboard_indexs.at(i).size(); j++) {
                    std::cout << keyboard_indexs.at(i).at(j) << std::endl;
                }

                std::cout << std::endl;
            }

            return true;
        }