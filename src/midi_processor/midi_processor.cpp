// includes -------------------------------------------------------------------
    #include "../../include/midi_processor/midi_processor.h"

// Namespace
    using namespace smf;

/*!
 * @brief Standalone Class for processing midi files and saving/loading mipi files. Contains various getters to store and provide data about midi/mipi files.
 * 
 * @author Connor McGannon
 * 
 * @details This class contains the following accesible data:
 * - **Channels**: A vector of all channels in the midi file
 * - **Instruments**: A vector of all instruments in the midi file
 * - **Notes**: A vector of all notes in the midi file, where each note is a vector of the note pitch and the note timing
 * - **Note Timings**: A vector of all note timings in the midi file, where each note timing is a vector of the note timing and the note duration
 * - **Note Durations**: A vector of all note durations in the midi file, where each note duration is a vector of the note duration and the note timing
 * - **Song Duration**: The duration of the song in seconds
 * - **Assigned Keys**: A vector of all assigned keys in the midi file, where each assigned key is a vector of the assigned key and the note timing 
 * - **Keyboard Values**: A vector of all keyboard values in the midi file, where each keyboard value is a vector of the keyboard value and the note timing
 * - **Keyboard Indexs**: A vector of all keyboard indexs in the midi file, where each keyboard index is a vector of the keyboard index and the note timing
 * 
 * @todo Improve filters
 */




// Core functions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // constructor ----------------------------------------------------------------
        /*! @brief Constructor that allocates internals
         *
         */
        MidiProcessor::MidiProcessor() 
        {
            // create midi
            midi = MidiFile();

            //!< current list of channels
            channels = std::vector<int>();

            //!< current list of instruments
            instruments = std::vector<int>();

            //!< current list of notes
            notes = std::vector<std::vector<int>>();

            //!< current list of note_timeStamps
            note_timeStamps = std::vector<std::vector<double>>();

            //!< current list of note_durations
            note_durations = std::vector<std::vector<double>>();

            //!< current song duration
            fileDuration = 0;

            //!< current list of assigned keys for each channel
            assigned_keys = std::vector<std::vector<int>>();

            //!< current list of keyboard values for each channel
            keyboard_values = std::vector<std::vector<int>>();

            //!< current list of keyboard indexs for each channel
            keyboard_indexs = std::vector<std::vector<int>>();

        }


    // destructor -----------------------------------------------------------------
        /*! @brief Destructor
         *
         */
        MidiProcessor::~MidiProcessor() 
        {

        }


// primary public functions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // -----------------------------------------------------------------------------
        /*! @brief processes a midi file by saving all instruments (and their channels) to a set of vectors
         *
         *  @param[in] std::string - midi_file_path The path to the midi file to be processed
         *  @param[in] std::string - json_file_name The name of the json file to be saved
         * 
         *  @return  int - returns 0 if there were no issues processing or saving the data. Any other number means an error occured
         * 
         *  @details This function opens the provided midi file, fully processes it, and saves all the data to a mipi file which can be reloaded. It is not neccesary to load a mipi file that was just processed as processing populates the variables.
         */
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
            if(!process_channel_notes_with_timings()) {
                return 4;
            }

            // process song duration
            if(!process_song_duration()) {
                return 5;
            }

            // filter chords
            if(!filter_chords()) {
                return 6;
            }

            // filter trills
            if(!filter_trills()) {
                return 7;
            }

            // filter overlapping notes
            if(!filter_overlapping_notes()) {
                return 8;
            }

            // trim note durations
            if(!trim_note_durations()) {
                return 9;
            }

            // assign keys
            if(!assign_keys()) {
                return 10;
            }

            // save data
            if(!save_midi_data(json_file_name)) {
                return 11;
            }

            // print data -------
            if(!debug_print_data()) {
                return -1;
            }

            return 0;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all channels
         *
         *  @return std::vector<int> - A vector of all channels in the midi file
         * 
         *  @details This function returns a vector of integers. Each integer is a channel number
         */
        std::vector<int> MidiProcessor::get_channels()
        {
            return channels;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all instruments
         *
         *  @return std::vector<int> - A vector of all instruments in the midi file
         * 
         *  @details This function returns a vector of integers. Each integer is an instrument number
         */
        std::vector<int> MidiProcessor::get_instruments()
        {
            return instruments;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all instrument names and their channels
         *
         *  @return std::vector<std::string> - A vector of all instrument names and their channel numbers
         * 
         *  @details This function returns a vector of strings. Each string is the name of an instrument and its channel number
         */
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


    // -----------------------------------------------------------------------------
        /*! @brief gets all notes
         *
         *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
         * 
         *  @details This function returns a vector of vectors of ints. Each vector contains notes for 1 channel as it is written in the midi file.
         */
        std::vector<std::vector<int>> MidiProcessor::get_channel_notes() 
        {
            return notes;
        }


    // ---------------------------------------------------------------------------
        /*! @brief gets all note timings
         *
         *  @return std::vector<std::vector<double>> - A vector of vectors of doubles.
         * 
         *  @details This function returns a vector of vectors of doubles. Each vector contains note time stamps for 1 channel. 
         */
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_timings() 
        {
            return note_timeStamps;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all note durations
         *
         *  @return std::vector<std::vector<double>> - A vector of vectors of doubles.
         * 
         *  @details This function returns a vector of vectors of doubles. Each vector contains note durations for 1 channel.
         */
        std::vector<std::vector<double>> MidiProcessor::get_channel_note_durations() 
        {
            return note_durations;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets the song duration
         *
         *  @return double - The duration of the song in seconds
         * 
         *  @details This function returns the duration of the song in seconds.                           
         */
        double MidiProcessor::get_song_duration() 
        {
            return fileDuration;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all assigned keys
         *
         *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
         * 
         *  @details This function returns a vector of vectors ints. Each vector contains assigned keys for 1 channel, which correspond to a note on the keyboard values vector.                           
         */
        std::vector<std::vector<int>> MidiProcessor::get_assigned_keys() 
        {
            return assigned_keys;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all keyboard values
         *
         *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
         * 
         *  @details This function returns a vector of vectors of ints. Each vector contains the 37 assigned keyboard values for 1 channel. The indexs of these keys match the corresponding indexs for the key position vector.                           
         */
        std::vector<std::vector<int>> MidiProcessor::get_keyboard_values() 
        {
            return keyboard_values;
        }


    // -----------------------------------------------------------------------------
        /*! @brief gets all keyboard indexs
         *
         *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
         * 
         *  @details This function returns a vector of vectors of ints. Each vector contains the indexs for the notes of the assigned keys as they appear in the keyboard values vector. The indexs of these keys match the corresponding indexs for the key position vector.                           
         */
        std::vector<std::vector<int>> MidiProcessor::get_keyboard_indexs() 
        {
            return keyboard_indexs;
        }


    // -----------------------------------------------------------------------------
        /*! @brief load JSON file
         *
         *  @param[in] std::string - the name of the mipi file that should be loaded into the class variables (e.g. "filename.mipi")
         * 
         *  @return bool - returns TRUE if file was successfully loaded and false otherwise
         * 
         *  @details This function clears the class variables and repopulates all the variables with the data from selected mipi file.
         */
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

    //  ----------------------------------------------------------------------------
        /*! @brief opens a selected midi file
         *
         *  @param[in] std::string - the midi file path that is to be accessed and processed
         *  
         *  @return bool - returns true if the midi file was successfully opened and read
         * 
         *  @details This function is called by the process_midi_file method. It opens and reads a midi file. It also runs a time analysis and a note on/off pairing method from the midifile library.
         */
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


    // -----------------------------------------------------------------------------
        /*! @brief processes and stores all instruments/channels
         *
         *  @return bool - returns true if all instruments for each channel were successfully extracted
         * 
         *  @details This function is called by the process_midi_file method. It extracts the instrument ID number for each channel from the midi file and stores it in the instruments vector
         */
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


    // -----------------------------------------------------------------------------
        /*! @brief processes and stores all notes that correspond to a specific channel
         * 
         *  @return bool - returns true if there were no errors while extracting channel notes  
         * 
         *  @details This function processes the notes for 1 channel at a time as specified by the input parameter storing them into the notes vector
         * 
         *  @deprecated This function has been functionally replaced by "process_channel_notes_with_timings" and is no longer used as of 26/03/2026
         */
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


    // -----------------------------------------------------------------------------
        /*! @brief processes and stores all notes and their timings and remove dud channels 
         *
         *  @return bool - returns true if there were no errors while extracting channel notes
         *  
         *  @details This function is called by the process_midi_file method. It processes the notes along with their timestamps and durations (in seconds) for, storing them into the notes, note_timestamps, and note_durations vectors respectively. It then removes any channels with no notes
         */
        bool MidiProcessor::process_channel_notes_with_timings()
        {
            //!< list of dud channels
            std::vector<int> dud_channels;

            // for each channel
            for(size_t k = 0; k < channels.size(); k++) {

                std::vector<int> channel_notes;
                std::vector<double> channel_note_durations;
                std::vector<double> note_on_timeStamps;

                int channel = channels.at(k);

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

                // if a channel has no notes remove it from the vectors of data else add to data
                if(channel_notes.size() == 0) {
                    // std::cout << "No notes found in channel " << channel << " index: " << i << std::endl;
                    dud_channels.push_back(k);
                }
                else {
                    notes.push_back(channel_notes);
                    note_timeStamps.push_back(note_on_timeStamps);
                    note_durations.push_back(channel_note_durations);
                }
            }

            // remove dud channels
            for(int i = dud_channels.size() - 1; i >= 0; i--) {

                channels.erase(channels.begin() + dud_channels.at(i));
                instruments.erase(instruments.begin() + dud_channels.at(i));

                // std::cout << "Removing dud channel: " << dud_channels.at(i) << std::endl;
            }

            if(channels.size() == 0) {
                std::cout << "No channels found" << std::endl;
                return false;
            }

            return true;
        }


    // -----------------------------------------------------------------------------
        /*! @brief filters chords down to only the root note
         *
         *  @version 1
         *  @date 02/04/2026
         *
         *  @return bool - returns true if the chord filtering algorithm was successful and the "notes", "note_timeStamps", and "note_durations" vectors remain the same length
         * 
         *  @details This function is called by the process_midi_file method. It removes overlapping timestamps and keeps either the highest or the lowest note on that time stamp depending on the instrument type (e.g. piano = high note, guitar = low note). The affected vectors are the "notes", "note_timeStamps", and "note_durations" vectors.
         */
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


        // -----------------------------------------------------------------------------
            /*! @brief filters trills and staggered chords down to only the first note
             *
             *  @version 1
             *  @date 07/05/2026
             *
             *  @return bool - returns true if the trill filtering algorithm was successful and the "notes", "note_timeStamps", and "note_durations" vectors remain the same length
             * 
             *  @details This function is called by the process_midi_file method. It removes trills and staggered chords and keeps either the first note played. The affected vectors are the "notes", "note_timeStamps", and "note_durations" vectors.
             */
        bool MidiProcessor::filter_trills()
        {
            // for each set of notes
            for(size_t i = 0; i < notes.size(); i++) {

                int length = notes.at(i).size();

                // if there are more than 1 note in the channel, filter for trills
                if(length > 1) {

                    // for each timestamp starting at index 1
                    int j = 1;
                    
                    while(j < length) {

                        // get timestamps (current and last kept)
                        double note_timeStamp = note_timeStamps.at(i).at(j);
                        double last_kept_timeStamp = note_timeStamps.at(i).at(j - 1);
                        
                        // get diff and check if it is less than the min note gap
                        double diff = note_timeStamp - last_kept_timeStamp;

                        if(diff < min_note_gap) {
                            
                            // remove note from arrays
                            notes.at(i).erase(notes.at(i).begin() + j);
                            note_durations.at(i).erase(note_durations.at(i).begin() + j);
                            note_timeStamps.at(i).erase(note_timeStamps.at(i).begin() + j);

                            // error checking
                            if(notes.at(i).size() != note_durations.at(i).size() || notes.at(i).size() != note_timeStamps.at(i).size()) {
                                std::cout << "Error erasing notes" << std::endl;
                                std::cout << "notes size: " << notes.at(i).size() << "notes duration size: " << note_durations.at(i).size() << "notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;
                            }

                            // reduce length
                            length--;
                        }
                        else {

                            // increment j
                            j++;
                        }
                    }
                }
            }

            return true;
        }


    // -----------------------------------------------------------------------------
        /*! @brief filters overlapping notes down to only the first note
         *
         *  @version 1
         *  @date 07/05/2026
         *
         *  @return bool - returns true if the overlapping note filtering algorithm was successful and the "notes", "note_timeStamps", and "note_durations" vectors remain the same length
         * 
         *  @details This function is called by the process_midi_file method. It removes notes that are played entirely within the duration of another note. The affected vectors are the "notes", "note_timeStamps", and "note_durations" vectors.
         */
        bool MidiProcessor::filter_overlapping_notes()
        {
            // for each set of notes
            for(size_t i = 0; i < notes.size(); i++) {

                int length = notes.at(i).size();

                // if there are more than 1 note in the channel, filter for trills
                if(length > 1) {

                    // for each timestamp starting at index 1
                    int j = 1;
                    
                    while(j < length) {

                        // get current and past timestamp and duration
                        double note_timeStamp = note_timeStamps.at(i).at(j);
                        double last_kept_timeStamp = note_timeStamps.at(i).at(j - 1);

                        double note_duration = note_durations.at(i).at(j);
                        double last_kept_duration = note_durations.at(i).at(j - 1);
                        
                        // calc note end times
                        double current_end_time = note_timeStamp + note_duration;
                        double last_kept_end_time = last_kept_timeStamp + last_kept_duration;

                        // check if note is within the bounds of the last kept note
                        if(current_end_time <= last_kept_end_time) {
                            
                            // remove note from arrays
                            notes.at(i).erase(notes.at(i).begin() + j);
                            note_durations.at(i).erase(note_durations.at(i).begin() + j);
                            note_timeStamps.at(i).erase(note_timeStamps.at(i).begin() + j);

                            // error checking
                            if(notes.at(i).size() != note_durations.at(i).size() || notes.at(i).size() != note_timeStamps.at(i).size()) {
                                std::cout << "Error erasing notes" << std::endl;
                                std::cout << "notes size: " << notes.at(i).size() << "notes duration size: " << note_durations.at(i).size() << "notes time stamp size: " << note_timeStamps.at(i).size() << std::endl;
                            }

                            // reduce length
                            length--;
                        }
                        else {

                            // increment j
                            j++;
                        }
                    }
                }
            }

            return true;
        }


    // -----------------------------------------------------------------------------
        /*! @brief processes song duration
         *
         *  @return bool - returns true if the song duration was successfully extracted
         *  
         *  @details This function is called by the process_midi_file method. It extracts the songs maximum duration by analysing each channels delta times and converting them into seconds. Stores the duration in the "duration" variable.                           
         */ 
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

    
    // -----------------------------------------------------------------------------
        /*! @brief key assignment function
         *
         *  @version 1
         *  @date 14/04/2026  
         *
         *  @return bool - returns true if the key assignment was successful and the assigned_keys vector is the same length as the "notes" vector after chord filtering.
         * 
         *  @details This function is called by the process_midi_file method. On a per channel basis, it finds the average note in a channel, shifts the keyboard values to put the average as close to the middle C as possible, then assigns each note a keyboard value of a matching pitch. If a note is outside the bounds of the keyboard values it shifts it up or down octaves until it is within the bounds.
         */
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


    // -----------------------------------------------------------------------------
        /*! @brief note duration trimming function
         *
         *  @version 1
         *  @date 05/05/2026
         * 
         *  @return bool - returns true if the note duration trimming was successful and the "note_durations" vector is the same length as the "notes" vector.
         * 
         *  @details This function is called by the process_midi_file method. It checks the durations of each note and whether they overlap with the next and if so it trims it down such that the note ends just before the next note starts.
         */
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
                    if((j + 1) < notes.at(i).size()) {
                        double next_note_timeStamp = note_timeStamps.at(i).at(j + 1);

                        // check diff between timestamps
                        double time_diff = next_note_timeStamp - this_note_timeStamp;

                        // check if note_duration is longer than next_note_duration
                        if(this_note_duration + min_note_duration_gap > time_diff) {

                            // convert to ms temporarilty to avoid rounding errors
                            this_note_duration = this_note_duration * 1000;
                            time_diff = time_diff * 1000;

                            // trim note_duration
                            this_note_duration = time_diff - min_note_duration_gap;

                            // convert back to s
                            this_note_duration = this_note_duration / 1000;

                            note_durations.at(i).at(j) = this_note_duration;
                        }
                    }
                }
            }

            return true;
        }


    // -----------------------------------------------------------------------------
        /*! @brief stores all data for current midi file in a storage file
         *
         *  @param[in] std::string - the name of the file the data will be saved to.
         * 
         *  @return bool - returns true if the data was successfully saved
         * 
         *  @details This function is called by the process_midi_file method. It creates a file of the specified name and saves all the data to it in a JSON format.
         */
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

        
    // -----------------------------------------------------------------------------
        /*! @brief prints all data to the console
         *
         *  @return bool - returns true if the data was successfully printed
         * 
         *  @details This function is called wherever the user wants to test the data. It prints all stored the data to the console.
         */
        bool MidiProcessor::debug_print_data()
        {

            // OPTIONAL: increase output precision
            std::cout << std::setprecision(15);

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