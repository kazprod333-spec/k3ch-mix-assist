#pragma once

#include "InsertPlanLibrary.h"
#include "MasterDsp.h"
#include "MasterPresets.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>

class MasterProcessor final : public juce::AudioProcessor,
                              private juce::AudioProcessorValueTreeState::Listener
{
public:
    MasterProcessor();
    ~MasterProcessor() override;

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
    double getTailLengthSeconds() const override { return 0.02; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    k3ch_master::InsertPlanLibrary insertLibrary;
    juce::AudioProcessorValueTreeState apvts;
    juce::String targetChannelName;
    juce::String lastStatus;

    bool isBypassed() const;
    bool isMasterMode() const;
    int currentInsertIndex() const;
    int currentTargetChannel() const;
    void getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const;
    float getLimiterGrDb() const;

    void setModeMaster (bool master, bool notifyHost = true);
    void loadMasterPreset (int index, bool notifyHost = true);
    juce::String currentPlanText() const;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout (const k3ch_master::InsertPlanLibrary&);
    void pushEngineFromParams();
    void setFloatParam (const juce::String& id, float value);
    void setChoiceParam (const juce::String& id, int index);
    void setBoolParam (const juce::String& id, bool value);

    k3ch_master::MasterDsp engine;
    bool applyingPreset = false;

    std::array<std::atomic<float>, 2> peak { { 0.0f, 0.0f } };
    std::array<std::atomic<float>, 2> rms { { 0.0f, 0.0f } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterProcessor)
};
