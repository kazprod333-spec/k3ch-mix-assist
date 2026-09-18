#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class OutputMeter final : public juce::Component, private juce::Timer
{
public:
    explicit OutputMeter (PresetsProcessor& processor);
    ~OutputMeter() override;
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    static float toDb (float linear) noexcept;
    void drawBar (juce::Graphics& g, juce::Rectangle<float> bounds, float db, float holdDb) const;

    PresetsProcessor& processor;
    float peakHoldL = -100.0f, peakHoldR = -100.0f;
    float peakDbL = -100.0f, peakDbR = -100.0f;
    float rmsDbL = -100.0f, rmsDbR = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputMeter)
};
