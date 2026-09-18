#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class LevelMeter final : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeter (MixAssistProcessor& processor);
    ~LevelMeter() override;

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    static float toDb (float linear) noexcept;
    void drawBar (juce::Graphics& g, juce::Rectangle<float> bounds, float db, float holdDb) const;

    MixAssistProcessor& processor;
    float peakHoldL = -100.0f, peakHoldR = -100.0f;
    float peakDbL = -100.0f, peakDbR = -100.0f;
    float rmsDbL = -100.0f, rmsDbR = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
