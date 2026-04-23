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

// class
class MidiProcessor
{
    public:

        // constructor & destructor -----------------------------------------------------

            MidiProcessor();
            ~MidiProcessor();

        // primary public functions -----------------------------------------------------

            // processes a midi file by saving all instruments (and their channels) to a vector and all notes belonging to a channel to a vector
            bool processMidiFile(std::string midi_file_path, std::string json_file_name);

            // gets all channels
            std::vector<int> get_channels();

            // gets all instruments
            std::vector<int> get_instruments();

            // gets all instrument names and their channels
            std::vector<std::string> get_instrument_names();

            // gets all notes that correspond to a specific channel
            std::vector<std::vector<int>> get_channel_notes();

            // gets all note timings that correspond to a specific channel
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