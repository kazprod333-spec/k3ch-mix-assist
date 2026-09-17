#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <array>

class MixAssistProcessor final : public juce::AudioProcessor
{
public:
    MixAssistProcessor();
    ~MixAssistProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "K3CH Mix Assist"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::String selectedSectionId { "session_checklist" };

    bool isBypassed() const;
    void getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void measure (const juce::AudioBuffer<float>& buffer);

    std::array<std::atomic<float>, 2> peak { { 0.0f, 0.0f } };
    std::array<std::atomic<float>, 2> rms  { { 0.0f, 0.0f } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixAssistProcessor)
};
