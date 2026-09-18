cmake_minimum_required(VERSION 3.22)

project(K3CHMixAssist
    VERSION 1.0.0
    DESCRIPTION "K3CH Plugin — studio mix-recipe helper (VST3)"
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
