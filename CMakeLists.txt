cmake_minimum_required(VERSION 3.22)

project(K3CH
    VERSION 1.0.0
    DESCRIPTION "Maison K3CH Production — K3CH Plugin + K3CH Presets + K3CH Master (VST3)"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Optional: cmake -DJUCE_PATH=/path/to/JUCE
set(JUCE_PATH "" CACHE PATH "Path to a local JUCE checkout (skips FetchContent)")

if (JUCE_PATH)
    add_subdirectory("${JUCE_PATH}" JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.15
        GIT_SHALLOW    TRUE)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(K3CHMixAssist
    COMPANY_NAME "Maison K3CH Production"
    BUNDLE_ID com.maisonk3ch.mixassist
    PLUGIN_MANUFACTURER_CODE K3ch
    PLUGIN_CODE Mxas
    FORMATS VST3 Standalone
    PRODUCT_NAME "K3CH Plugin"
    PLUGIN_NAME "K3CH Plugin"
    DESCRIPTION "Studio mix recipes, session checklist, and starting-point chains (Maison K3CH Production, Alger)"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

juce_add_binary_data(K3CHMixAssistData
    HEADER_NAME BinaryData.h
    NAMESPACE BinaryData
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/mix-knowledge.json")

target_sources(K3CHMixAssist
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Knowledge/MixKnowledge.cpp
        Source/Knowledge/MixKnowledge.h
        Source/UI/K3chLookAndFeel.cpp
        Source/UI/K3chLookAndFeel.h
        Source/UI/LevelMeter.cpp
        Source/UI/LevelMeter.h
        Source/UI/RecipeView.cpp
        Source/UI/RecipeView.h)

target_include_directories(K3CHMixAssist
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source")

target_compile_definitions(K3CHMixAssist
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        K3CH_VERSION="${PROJECT_VERSION}")

if (MSVC)
    target_compile_options(K3CHMixAssist PRIVATE /utf-8 /W4)
else()
    target_compile_options(K3CHMixAssist PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(K3CHMixAssist
    PRIVATE
        K3CHMixAssistData
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Product 2 — K3CH Presets (vocal chain + internal sends A/B)
# ---------------------------------------------------------------------------

juce_add_plugin(K3CHPresets
    COMPANY_NAME "Maison K3CH Production"
    BUNDLE_ID com.maisonk3ch.presets
    PLUGIN_MANUFACTURER_CODE K3ch
    PLUGIN_CODE K3pr
    FORMATS VST3 Standalone
    PRODUCT_NAME "K3CH Presets"
    PLUGIN_NAME "K3CH Presets"
    DESCRIPTION "Vocal-chain preset loader and internal multi-FX sends (Maison K3CH Production, Alger)"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

juce_add_binary_data(K3CHPresetsData
    HEADER_NAME PresetsBinaryData.h
    NAMESPACE PresetsBinaryData
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/presets-runtime.json")

target_sources(K3CHPresets
    PRIVATE
        Source/Presets/PluginProcessor.cpp
        Source/Presets/PluginProcessor.h
        Source/Presets/PluginEditor.cpp
        Source/Presets/PluginEditor.h
        Source/Presets/PresetLibrary.cpp
        Source/Presets/PresetLibrary.h
        Source/Presets/CommandParser.cpp
        Source/Presets/CommandParser.h
        Source/Presets/DspEngine.cpp
        Source/Presets/DspEngine.h
        Source/Presets/UI/OutputMeter.cpp
        Source/Presets/UI/OutputMeter.h
        Source/UI/K3chLookAndFeel.cpp
        Source/UI/K3chLookAndFeel.h)

target_include_directories(K3CHPresets
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Presets")

target_compile_definitions(K3CHPresets
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        K3CH_VERSION="${PROJECT_VERSION}")

if (MSVC)
    target_compile_options(K3CHPresets PRIVATE /utf-8 /W4)
else()
    target_compile_options(K3CHPresets PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(K3CHPresets
    PRIVATE
        K3CHPresetsData
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

add_executable(K3CHPresetsSmoke
    Source/Presets/smoke_test.cpp
    Source/Presets/DspEngine.cpp)
target_include_directories(K3CHPresetsSmoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Presets")
target_compile_features(K3CHPresetsSmoke PRIVATE cxx_std_20)
if (MSVC)
    target_compile_options(K3CHPresetsSmoke PRIVATE /utf-8 /W4)
else()
    target_compile_options(K3CHPresetsSmoke PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Product 3 — K3CH Master (Studio One 8 master bus + FX Chain plans)
# ---------------------------------------------------------------------------

juce_add_plugin(K3CHMaster
    COMPANY_NAME "Maison K3CH Production"
    BUNDLE_ID com.maisonk3ch.master
    PLUGIN_MANUFACTURER_CODE K3ch
    PLUGIN_CODE K3ms
    FORMATS VST3 Standalone
    PRODUCT_NAME "K3CH Master"
    PLUGIN_NAME "K3CH Master"
    DESCRIPTION "Mastering chain DSP + Studio One FX Chain plans (Maison K3CH Production, Alger)"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

juce_add_binary_data(K3CHMasterData
    HEADER_NAME MasterBinaryData.h
    NAMESPACE MasterBinaryData
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/master-inserts.json")

target_sources(K3CHMaster
    PRIVATE
        Source/Master/PluginProcessor.cpp
        Source/Master/PluginProcessor.h
        Source/Master/PluginEditor.cpp
        Source/Master/PluginEditor.h
        Source/Master/MasterDsp.cpp
        Source/Master/MasterDsp.h
        Source/Master/MasterPresets.cpp
        Source/Master/MasterPresets.h
        Source/Master/InsertPlanLibrary.cpp
        Source/Master/InsertPlanLibrary.h
        Source/Master/UI/OutputMeter.cpp
        Source/Master/UI/OutputMeter.h
        Source/Master/UI/Controls.h
        Source/UI/K3chLookAndFeel.cpp
        Source/UI/K3chLookAndFeel.h)

target_include_directories(K3CHMaster
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Master")

target_compile_definitions(K3CHMaster
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        K3CH_VERSION="${PROJECT_VERSION}")

if (MSVC)
    target_compile_options(K3CHMaster PRIVATE /utf-8 /W4)
else()
    target_compile_options(K3CHMaster PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(K3CHMaster
    PRIVATE
        K3CHMasterData
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

add_executable(K3CHMasterSmoke
    Source/Master/smoke_test.cpp
    Source/Master/MasterDsp.cpp
    Source/Master/MasterPresets.cpp)
target_include_directories(K3CHMasterSmoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Master")
target_compile_features(K3CHMasterSmoke PRIVATE cxx_std_20)
if (MSVC)
    target_compile_options(K3CHMasterSmoke PRIVATE /utf-8 /W4)
else()
    target_compile_options(K3CHMasterSmoke PRIVATE -Wall -Wextra -Wpedantic)
endif()
