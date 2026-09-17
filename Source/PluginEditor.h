#pragma once

#include "PluginProcessor.h"
#include "Knowledge/MixKnowledge.h"
#include "UI/K3chLookAndFeel.h"
#include "UI/LevelMeter.h"
#include "UI/RecipeView.h"

class MixAssistEditor final : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    explicit MixAssistEditor (MixAssistProcessor&);
    ~MixAssistEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildNav();
    void showSection (const juce::String& id);
    void applyFilter();
    void copyRecipe();
    void showAbout();

    MixAssistProcessor& processor;
    k3ch::MixKnowledge knowledge;
    k3ch::K3chLookAndFeel lnf;

    juce::TextEditor search;
    juce::OwnedArray<juce::TextButton> navButtons;
    juce::Viewport viewport;
    RecipeView recipeView;
    LevelMeter meter;
    juce::TextButton bypassButton { "BYPASS" };
    juce::TextButton copyButton { "Copier la recette" };
    juce::TextButton aboutButton { "A propos" };
    juce::Label statusLabel;
    juce::String currentSectionId;
    juce::String statusUntilClear;
    int statusTicks = 0;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixAssistEditor)
};
