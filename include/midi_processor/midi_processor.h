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
#include <filesystem>

// MidiFile library
#include "midiLibrary/include/Binasc.h"
#include "midiLibrary/include/MidiFile.h"
#include "midiLibrary/include/MidiEvent.h"
#include "midiLibrary/include/MidiMessage.h"
#include "midiLibrary/include/MidiEventList.h"
#include "midiLibrary/include/Options.h"
#include "../external/json.hpp"




using namespace smf;
using json = nlohmann::json;

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
 */
class MidiProcessor
{
    public:

        // constructor & destructor -----------------------------------------------------
            
            /*! @brief Constructor that allocates internals
             *
             */
            MidiProcessor();


            /*! @brief Destructor
             *
             */
            ~MidiProcessor();

        // primary public functions -----------------------------------------------------

            /*! @brief processes a midi file by saving all instruments (and their channels) to a set of vectors
             *
             *  @param midi_file_path The path to the midi file to be processed
             * 
             *  @param json_file_name The name of the json file to be saved
             */
            bool processMidiFile(std::string midi_file_path, std::string json_file_name);


            /*! @brief gets all channels
             *
             *  @return std::vector<int> - A vector of all channels in the midi file
             * 
             *  @details This function returns a vector of integers. Each integer is a channel number
             */
            std::vector<int> get_channels();


            /*! @brief gets all instruments
             *
             *  @return std::vector<int> - A vector of all instruments in the midi file
             * 
             *  @details This function returns a vector of integers. Each integer is an instrument number
             */
            std::vector<int> get_instruments();


            /*! @brief gets all instrument names and their channels
             *
             *  @return std::vector<std::string> - A vector of all instrument names and their channel numbers
             * 
             *  @details This function returns a vector of strings. Each string is the name of an instrument and its channel number
             */
            std::vector<std::string> get_instrument_names();


            /*! @brief gets all notes that correspond to a specific channel
             *
             *  @return std::vector<std::vector<int>> - A vector of vectors of ints. Each vector contains notes for 1 channel as written in the midi file.
             * 
             *  @details This function returns a vector of vectors. Each vector contains a note as it is written in the midi file.
             */
            std::vector<std::vector<int>> get_channel_notes();

            /*! @brief gets all note timings that correspond to a specific channel
             *
             *  @return std::vector<std::vector<double>> - A vector of vectors of doubles. Each vector contains note timestamps for 1 channel.
             * 
             *  @details This function returns a vector of vectors. Each vector contains a note timing as it is written in the midi file.
             */
            std::vector<std::vector<double>> get_channel_note_timings();

            // gets all note durations that correspond to a specific channel
            std::vector<std::vector<double>> get_channel_note_durations();

            // get song duration
            double get_song_duration();

            // get assigned keys
            std::vector<std::vector<int>> get_assigned_keys();

            // get keyboard values
            std::vector<std::vector<int>> get_keyboard_values();

            // load JSON file
            bool load_json_file(std::string json_file_path);

        // public variables -------------------------------------------------------------

    private:

        // primary private functions ----------------------------------------------------

            // opens a selected midi file
            bool open_file(std::string midi_file_path);

            // processes and stores all instruments/channels
            bool process_instruments();

            // processes and stores all notes that correspond to a specific channel
            bool process_channel_notes(int channel);

            // processes and stores all notes and their timings that correspond to a specific channel
            bool process_channel_notes_with_timings(int channel);

            // filters chords down to only the root note
            bool filter_chords();

            // process song duration
            bool process_song_duration();

            // key assignment function
            bool assign_keys();

            // stores all data for current midi file in a storage file
            bool save_midi_data(std::string file_name);


        // private variables ------------------------------------------------------------

            // MidiFile library
            MidiFile midi;

            // home directory
            const char* homeDir = getenv("HOME");

            // current list of channels
            std::vector<int> channels;

            // current list of instruments
            std::vector<int> instruments;

            // current list of notes
            std::vector<std::vector<int>> notes;

            // current list of note_timeStamps
            std::vector<std::vector<double>> note_timeStamps;

            // current list of note_durations
            std::vector<std::vector<double>> note_durations;

            // current song duration
            double fileDuration;

            // current list of assigned keys for each channel
            std::vector<std::vector<int>> assigned_keys;

            // current list of keyboard values for each channel
            std::vector<std::vector<int>> keyboard_values;
    
};


#endif