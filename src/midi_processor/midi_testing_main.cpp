// test file for midi reading class

// includes
#include <iostream>
#include <vector>
#include <string>
#include "../../include/midi_processor/midi_processor.h"

// test functions
int test_notes();
int test_process();
int test_get_instrument_names();
int test_save_and_load_mipi_data();
int test_load_mipi_data();


int main() {
    
    std::cout << "Testing midi Processor class - v0.1" << std::endl;

    // Run test
    test_load_mipi_data();

    // close file
    return 0;
}


int test_process() {

    // create midi
    MidiProcessor midi;

    // twinkle twinkle little star
    // std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid";
    // pokemon
    // std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/Driftveil_City_(Pokémon B1W1).mid";
    // mary had a little lamb
    // std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/mary-had-a-little-lamb.mid";
    // rick roll
    // std::string midi_file_path = "/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/Never-Gonna-Give-You-Up-3.mid";

    // open file
    int error = midi.processMidiFile(midi_file_path, "test_name.mipi"); 
    if(error != 0) {
        std::cout << "Error opening midi file: " << error << std::endl;
        return 0;
    }

    return 0;
}


int test_get_instrument_names() {

    // create midi
    MidiProcessor midi;

    // open file
    if(!midi.processMidiFile("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid", "test_name.mipi")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::vector<std::string> instrument_names;

    instrument_names = midi.get_instrument_names();

    // display notes
    std::cout << "Instrument names: " << std::endl;
    for(size_t i = 0; i < instrument_names.size(); i++) {
        std::cout << instrument_names[i] << std::endl;
    }

    return 0;
}



int test_save_and_load_mipi_data() {

    // create midi
    MidiProcessor midi;

    // open file 1
    std::cout << "Opening midi file 1" << std::endl;
    if(!midi.processMidiFile("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/mary-had-a-little-lamb.mid", "mary.mipi")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    // open file 2
    std::cout << "Opening midi file 2 (overidding data)" << std::endl;
    if(!midi.processMidiFile("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/midi_files/twinkle-twinkle-little-star.mid", "twinkle.mipi")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    // load the data for file 1

    std::cout << "Loading midi data for file 1" << std::endl;
    midi.load_json_file("mary.mipi");

    return 0;
}


int test_load_mipi_data() {

    // create midi
    MidiProcessor midi;

    // open file 1
    std::cout << "Opening midi file 1" << std::endl;
    if(!midi.load_json_file("/home/connor/git/robo-studio-2/RoboticStudio2-JAMC/mipi_files/max_min.mipi")) {
        std::cout << "Error opening midi file" << std::endl;
        return 0;
    }

    auto data = midi.get_channel_notes();

    // display notes
    for(size_t i = 0; i < data.size(); i++) {
        std::cout << "Channel: " << i << std::endl;
        for(size_t j = 0; j < data.at(i).size(); j++) {
            std::cout << data.at(i).at(j) << std::endl;
        }
    }

    return 0;
    
}