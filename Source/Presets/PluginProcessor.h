#pragma once

#include "CommandParser.h"
#include "DspEngine.h"
#include "PresetLibrary.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>

class PresetsProcessor final : public juce::AudioProcessor,
                               private juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetsProcessor();
    ~PresetsProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    k3ch_presets::PresetLibrary library;
    juce::AudioProcessorValueTreeState apvts;

    bool isBypassed() const;
    void getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const;
    float getCompGrDb() const;

    k3ch_presets::Command applyCommand (const juce::String& text);
    void loadVocalIndex (int index, bool notifyHost = true);
    void loadFxIndex (int sendSlot, int index, bool applyDefaultLevel = true, bool notifyHost = true);

    juce::String lastStatus;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout (const k3ch_presets::PresetLibrary&);
    void pushEngineFromParams();
    void setFloatParam (const juce::String& id, float value, bool notifyHost);
    void setChoiceParam (const juce::String& id, int index, bool notifyHost);
    void setBoolParam (const juce::String& id, bool value, bool notifyHost);
    void writeSendParams (int sendSlot, const k3ch_presets::SendSettings& s, bool notifyHost);
    juce::String sendParamId (int sendSlot, const char* suffix) const;

    k3ch_presets::DspEngine engine;
    k3ch_presets::CommandParser parser;
    bool applyingPreset = false;

    std::array<std::atomic<float>, 2> peak { { 0.0f, 0.0f } };
    std::array<std::atomic<float>, 2> rms { { 0.0f, 0.0f } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsProcessor)
};
