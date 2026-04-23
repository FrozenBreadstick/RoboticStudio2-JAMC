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
 * 
 * @todo Add more documentation, make getter for vector of vectors of ints that stores index positions of notes in the keyboard values vectors
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
             *  @param[in] std::string - midi_file_path The path to the midi file to be processed
             *  @param[in] std::string - json_file_name The name of the json file to be saved
             * 
             *  @return bool - returns true if there were no issues processing or saving the data.
             * 
             *  @details This function opens the provided midi file, fully processes it, and saves all the data to a mipi file which can be reloaded. It is not neccesary to load a mipi file that was just processed as processing populates the variables.
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


            /*! @brief gets all notes
             *
             *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
             * 
             *  @details This function returns a vector of vectors of ints. Each vector contains notes for 1 channel as it is written in the midi file.
             */
            std::vector<std::vector<int>> get_channel_notes();


            /*! @brief gets all note timings
             *
             *  @return std::vector<std::vector<double>> - A vector of vectors of doubles.
             * 
             *  @details This function returns a vector of vectors of doubles. Each vector contains note time stamps for 1 channel.
             */
            std::vector<std::vector<double>> get_channel_note_timings();


            /*! @brief gets all note durations
             *
             *  @return std::vector<std::vector<double>> - A vector of vectors of doubles.
             * 
             *  @details This function returns a vector of vectors of doubles. Each vector contains note durations for 1 channel.
             */
            std::vector<std::vector<double>> get_channel_note_durations();


            /*! @brief gets the song duration
             *
             *  @return double - The duration of the song in seconds
             * 
             *  @details This function returns the duration of the song in seconds.
             */
            double get_song_duration();


            /*! @brief gets all assigned keys
             *
             *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
             * 
             *  @details This function returns a vector of vectors ints. Each vector contains assigned keys for 1 channel, which correspond to a note on the keyboard values vector.
             */
            std::vector<std::vector<int>> get_assigned_keys();


            /*! @brief get keyboard values
             *
             *  @return std::vector<std::vector<int>> - A vector of vectors of ints.
             * 
             *  @details This function returns a vector of vectors of ints. Each vector contains the 37 assigned keyboard values for 1 channel. The indexs of these keys match the corresponding indexs for the key position vector.
             */ 
            std::vector<std::vector<int>> get_keyboard_values();


            /*! @brief load JSON file
             *
             *  @param[in] std::string - the name of the mipi file that should be loaded into the class variables (e.g. "filename.mipi")
             * 
             *  @return bool - returns TRUE if file was successfully loaded and false otherwise
             * 
             *  @details This function clears the class variables and repopulates all the variables with the data from selected mipi file.
             */ 
            bool load_json_file(std::string json_file_path);

        // public variables -------------------------------------------------------------

    private:

        // primary private functions ----------------------------------------------------

            /*! @brief opens a selected midi file
             *
             *  @param[in] std::string - the midi file path that is to be accessed and processed
             *  
             *  @return bool - returns true if the midi file was successfully opened and read
             * 
             *  @details This function is called by the process_midi_file method. It opens and reads a midi file. It also runs a time analysis and a note on/off pairing method from the midifile library.
             */ 
            bool open_file(std::string midi_file_path);


            /*! @brief processes and stores all instruments/channels
             *
             *  @return bool - returns true if all instruments for each channel were successfully extracted
             * 
             *  @details This function is called by the process_midi_file method. It extracts the instrument ID number for each channel from the midi file and stores it in the instruments vector
             */ 
            bool process_instruments();


            /*! @brief processes and stores all notes that correspond to a specific channel
             *
             *  @param[in] int - the channel number of the notes that are to be processed and stored
             * 
             *  @return bool - returns true if there were no errors while extracting channel notes  
             * 
             *  @details This function processes the notes for 1 channel at a time as specified by the input parameter storing them into the notes vector
             * 
             *  @deprecated This function has been functionally replaced by "process_channel_notes_with_timings" and is no longer used as of 26/03/2026
             */ 
            bool process_channel_notes(int channel);


            /*! @brief processes and stores all notes and their timings that correspond to a specific channel  
             *
             *  @param[in] int - the channel number of the note timestamps that are to be processed and stored.
             * 
             *  @return bool - returns true if there were no errors while extracting channel notes
             *  
             *  @details This function is called by the process_midi_file method. It processes the notes along with their timestamps and durations (in seconds) for 1 channel at a time as specified by the input parameter, storing them into the notes, note_timestamps, and note_durations vectors respectively
             */ 
            bool process_channel_notes_with_timings(int channel);


            /*! @brief filters chords down to only the root note
             *
             *  @version 1
             *  @date 02/04/2026
             *
             *  @return bool - returns true if the chord filtering algorithm was successful and the "notes", "note_timeStamps", and "note_durations" vectors remain the same length
             * 
             *  @details This function is called by the process_midi_file method. It removes overlapping timestamps and keeps either the highest or the lowest note on that time stamp depending on the instrument type (e.g. piano = high note, guitar = low note). The affected vectors are the "notes", "note_timeStamps", and "note_durations" vectors.
             */ 
            bool filter_chords();


            /*! @brief process song duration
             *
             *  @return bool - returns true if the song duration was successfully extracted
             *  
             *  @details This function is called by the process_midi_file method. It extracts the songs maximum duration by analysing each channels delta times and converting them into seconds. Stores the duration in the "duration" variable.
             */ 
            bool process_song_duration();


            /*! @brief key assignment function
             *
             *  @version 1
             *  @date 14/04/2026  
             *
             *  @return bool - returns true if the key assignment was successful and the assigned_keys vector is the same length as the "notes" vector after chord filtering.
             * 
             *  @details This function is called by the process_midi_file method. On a per channel basis, it finds the average note in a channel, shifts the keyboard values to put the average as close to the middle C as possible, then assigns each note a keyboard value of a matching pitch. If a note is outside the bounds of the keyboard values it shifts it up or down octaves until it is within the bounds.
             */ 
            bool assign_keys();


            /*! @brief stores all data for current midi file in a storage file
             *
             *  @param[in] std::string - the name of the file the data will be saved to.
             * 
             *  @return bool - returns true if the data was successfully saved
             * 
             *  @details This function is called by the process_midi_file method. It creates a file of the specified name and saves all the data to it in a JSON format.
             */
            bool save_midi_data(std::string file_name);


        // private variables ------------------------------------------------------------

            //!< MidiFile library
            MidiFile midi; 

            //!< home directory
            const char* homeDir = getenv("HOME");

            //!< current list of channels
            std::vector<int> channels;

            //!< current list of instruments
            std::vector<int> instruments;

            //!< current list of notes
            std::vector<std::vector<int>> notes;

            //!< current list of note_timeStamps
            std::vector<std::vector<double>> note_timeStamps;

            //!< current list of note_durations
            std::vector<std::vector<double>> note_durations;

            //!< current song duration
            double fileDuration;

            //!< current list of assigned keys for each channel
            std::vector<std::vector<int>> assigned_keys;

            //!< current list of keyboard values for each channel
            std::vector<std::vector<int>> keyboard_values;
    
};


#endif