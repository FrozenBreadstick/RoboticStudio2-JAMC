/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "MIPI - Physical Interface for MIDI Files", "index.html", [
    [ "Main Page Overview", "index.html#main_overview", null ],
    [ "Node and Library Descriptions", "index.html#node_descriptions", [
      [ "UI - Human 2 Robot Interaction", "index.html#ui_section", [
        [ "Layout & User Experience", "index.html#ui_layout", null ],
        [ "Technical Implementation", "index.html#ui_tech", null ],
        [ "ROS 2 Interface Mapping", "index.html#ui_services", null ]
      ] ],
      [ "MIDI Processing - For parsing and converting midi files to a readable format by our system", "index.html#midi_section", null ],
      [ "Controller - Control Engine for UR3e Piano Playing", "index.html#controller_section", [
        [ "Behaviour", "index.html#Behaviour", null ]
      ] ],
      [ "Perception & Computer Vision - Let's the robot see", "index.html#perception_section", null ],
      [ "External Libraries - JSON, MIDI Parsing, etc", "index.html#external_section", null ]
    ] ],
    [ "Code Behaviour and Usage", "index.html#code_behaviour", [
      [ "Build Instructions", "index.html#build_method", null ],
      [ "Launch Files - Node Startup and Execution (Usage)", "index.html#launch_method", null ]
    ] ],
    [ "UI_README", "md_UI_README.html", [
      [ "HOW TO USE THE PIANO UI CLASS", "md_UI_README.html#ui_usage", [
        [ "How to Initialize the UI Node", "md_UI_README.html#autotoc_md0", [
          [ "1. <strong>Initialize ROS 2</strong>", "md_UI_README.html#autotoc_md1", null ],
          [ "2. <strong>Instantiate the UI</strong>", "md_UI_README.html#autotoc_md2", null ],
          [ "3. <strong>Show the window and spin the node</strong>", "md_UI_README.html#autotoc_md3", null ]
        ] ]
      ] ],
      [ "How to Interact with the UI Features", "md_UI_README.html#autotoc_md5", [
        [ "Step 1: Processing a New MIDI File", "md_UI_README.html#autotoc_md6", null ],
        [ "Step 2: Loading an Existing .mipi File", "md_UI_README.html#autotoc_md7", null ],
        [ "Step 3: Selecting a Channel", "md_UI_README.html#autotoc_md8", null ],
        [ "Step 4: Controlling Playback", "md_UI_README.html#autotoc_md9", null ]
      ] ],
      [ "Technical Interface Reference", "md_UI_README.html#autotoc_md11", [
        [ "Service Clients Used", "md_UI_README.html#autotoc_md12", null ],
        [ "Camera Subscription", "md_UI_README.html#autotoc_md13", null ]
      ] ]
    ] ],
    [ "UR_README", "md_UR_README.html", [
      [ "How to use the Controller Class", "md_UR_README.html#autotoc_md14", [
        [ "1. Installation", "md_UR_README.html#autotoc_md15", null ],
        [ "@code{bash}", "md_UR_README.html#autotoc_md17", null ],
        [ "2. Physical Robot Setup", "md_UR_README.html#autotoc_md19", null ],
        [ "3. Software Setup", "md_UR_README.html#autotoc_md21", [
          [ "Automatic", "md_UR_README.html#autotoc_md22", [
            [ "UR Driver Installtion", "md_UR_README.html#autotoc_md16", null ],
            [ "Motion Planner Installation", "md_UR_README.html#autotoc_md18", null ],
            [ "On Teach Pendant", "md_UR_README.html#autotoc_md20", null ],
            [ "Running the Built in Launch File", "md_UR_README.html#autotoc_md23", null ]
          ] ],
          [ "Manual", "md_UR_README.html#autotoc_md25", [
            [ "Running the Driver", "md_UR_README.html#autotoc_md26", null ]
          ] ]
        ] ],
        [ "@code{bash}", "md_UR_README.html#autotoc_md27", null ],
        [ "@code{bash}", "md_UR_README.html#autotoc_md29", null ],
        [ "Extracting Robot Calibration", "md_UR_README.html#autotoc_md30", null ],
        [ "4. Using the Controller Class in Code", "md_UR_README.html#autotoc_md31", null ]
      ] ]
    ] ],
    [ "MIDI_README", "md_MIDI_README.html", [
      [ "HOW TO USE THE MIDI PROCESSOR CLASS", "md_MIDI_README.html#autotoc_md34", [
        [ "how to use for processing files", "md_MIDI_README.html#autotoc_md35", null ],
        [ "how to use for loading mipi files and getting processed data", "md_MIDI_README.html#autotoc_md36", null ]
      ] ]
    ] ],
    [ "AI_README", "md_AI_README.html", [
      [ "How to setup realsense camera", "md_AI_README.html#autotoc_md37", null ]
    ] ],
    [ "Todo List", "todo.html", null ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", "functions_type" ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"Binasc_8cpp.html",
"classbasic__json.html#a3af74b651da5642cd8b77a3ecc635331",
"classdetail_1_1binary__reader.html#a116e37cc0712b400ea2d6a9c4abf1cac",
"classdetail_1_1json__reverse__iterator.html#a05c6be3b2139e3157a1b2cb7f458d54f",
"classdetail_1_1primitive__iterator__t.html#af58da4713ea9010912f3da6b22aeee51",
"classsmf_1_1MidiFile.html#a18ea982955addb049640af5165759941",
"classsmf_1_1MidiMessage.html#aa1474c9b14abb5cc8b3b2aa655ec283b",
"functions_vars_x.html",
"json_8hpp.html#a77cca410ac9e251e0ff2847df09e03d9",
"json_8hpp.html#af80e4e59be493e62842fb40354351e8b",
"namespacedetail.html#ae785f1c6c99c714463b625da13a75dad",
"structdetail_1_1is__json__iterator__of_3_01BasicJsonType_00_01typename_01BasicJsonType_1_1const__iterator_01_4.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';