#pragma once

#include "PluginProcessor.h"
#include "UI/OutputMeter.h"
#include "UI/K3chLookAndFeel.h"

class PresetsEditor final : public juce::AudioProcessorEditor,
                            private juce::Timer
{
public:
    explicit PresetsEditor (PresetsProcessor&);
    ~PresetsEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void applyCommand();
    void applySelectedChain();
    void placeFxOnSelectedSend();
    void copyFlInsertPlan();
    void syncCombosFromProcessor();
    void updateReadouts();
    void showAbout();
    void styleReadOnly (juce::TextEditor& editor);

    juce::Rectangle<int> chainCard, planCard, placeCard, sendACard, sendBCard;

    PresetsProcessor& processor;
    k3ch::K3chLookAndFeel lnf;
    OutputMeter meter;

    juce::TextEditor commandBox;
    juce::TextButton applyButton { "Appliquer" };
    juce::Label statusLabel;
    juce::Label constraintNote;
    juce::TextButton bypassButton { "BYPASS" };
    juce::TextButton aboutButton { "About" };

    juce::Label chainLabel;
    juce::ComboBox vocalBox;
    juce::TextButton applyChainButton;
    juce::Label chainHint;
    juce::TextEditor chainView;

    juce::Label planHint;
    juce::TextButton copyPlanButton;
    juce::TextEditor planView;

    juce::Label sendTargetLabel;
    juce::ComboBox sendTargetBox;
    juce::Label fxPresetLabel;
    juce::ComboBox fxPresetBox;
    juce::TextButton placeButton { "Placer" };
    juce::Label placeHint;
    juce::Label fxChoiceReadout;

    juce::Label sendAName, sendBName;
    juce::Slider sendALevel, sendBLevel;
    juce::Label sendAReadout, sendBReadout;
    juce::Slider dryWet, returnMix, outputDb;
    juce::Label dryLabel, returnLabel, outLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sendAAttach, sendBAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryAttach, retAttach, outAttach;

    bool ignoreCombo = false;
    int statusTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsEditor)
};
