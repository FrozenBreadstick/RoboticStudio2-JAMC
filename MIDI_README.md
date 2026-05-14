# How to use the Midi Processor Class

<hr style="border: none; border-top: 3px double #558bff;">

see midi_testing_main.cpp for an example of how to use the class for processing


## How to use for processing files

step 1: instantiate the class

```bash
MidiProcessor midi;
```

step 2: call the process method with the path to the midi file you want to process AND the file name you want to save the processed midi file as

```bash
path_to_midi_file = "path/to/midi/file.mid";
file_name = "file_name.mipi";

midi.process(path_to_midi_file, file_name);
```

note: When a file is processed, it will be saved automatically under whatever file name you specify.


## how to use for loading mipi files and getting processed data

step 1: instantiate the class

```bash
MidiProcessor midi;
```

step 2: call the load method with the name of the mipi file you want to load

```bash
file_name = "file_name.mipi";

midi.load(file_name);
```

step 3: call the get method with the name of the midi data you want to get (all examples below)

```bash
// returns a std::vector<int> of all channels
midi.get_channels();

// returns a std::vector<int> of all instruments
midi.get_instruments();

// returns a std::vector<std::string> of all instrument names and their channel numbers
midi.get_instrument_names();

// returns a std::vector<std::vector<int>> of all notes for each channel
midi.get_channel_notes();

// returns a std::vector<std::vector<double>> of all note time stamps for each channel
midi.get_channel_note_timings();

// returns a std::vector<std::vector<double>> of all note durations for each channel
midi.get_channel_note_durations();

// returns a double of the song duration
midi.get_song_duration();

// returns a std::vector<std::vector<int>> of all assigned keys for each channel
midi.get_assigned_keys();

// returns a std::vector<std::vector<int>> of all keyboard values for each channel
midi.get_keyboard_values();

// returns a std::vector<std::vector<int>> of all keyboard indexs for each channel
midi.get_keyboard_indexs();
```

note: the indexs for each vector are corresponding vectors of the same channel. For example, if a song has 2 channels, then index 0 for all vectors will be for the one channel and index 1 for all vectors will be for the next channel. This DOES NOT mean "midi channel 1" is in index position 0 or 1, all data relating to midi channel 1 will have the same index position in all vectors.
