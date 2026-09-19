#pragma once

#include "PluginProcessor.h"
#include "UI/Controls.h"
#include "UI/OutputMeter.h"
#include "UI/K3chLookAndFeel.h"

class MasterEditor final : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit MasterEditor (MasterProcessor&);
    ~MasterEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshMode();
    void applySelectedMasterPreset();
    void copyPlan();
    void updatePlanView();
    void showAbout();
    void styleReadOnly (juce::TextEditor& editor);
    void addTo (juce::Component& page, juce::Component& child);
    void layoutMasterPage();
    void layoutInsertsPage();

    MasterProcessor& processor;
    k3ch::K3chLookAndFeel lnf;
    MasterOutputMeter meter;

    juce::TextButton insertsModeBtn, masterModeBtn;
    juce::TextButton bypassButton { "BYPASS" };
    juce::TextButton aboutButton;
    juce::Label constraintNote;
    juce::Label statusLabel;

    juce::Component pageMaster, pageInserts;

    juce::Label presetLabel;
    juce::ComboBox masterPresetBox;
    juce::TextButton applyPresetBtn;

    juce::TabbedComponent masterTabs { juce::TabbedButtonBar::TabsAtTop };
    LayoutPage pageEq, pageSat, pageDyn;

    juce::Label eqLowTitle, eqMidTitle, eqHighTitle;
    ControlRow inputDb, outputDb;
    ControlRow lowHz, lowG, highHz, highG;
    ToggleRow midOn;
    ControlRow midHz, midG, midQ;

    ToggleRow satOn;
    ControlRow satDrv, satMix, width, bassMono;

    ToggleRow clipOn, limOn, tpOn;
    ControlRow clipCeil, limCeil, limRel, limLook, tpCeil;
    GrMeter grMeter;

    juce::Label insertListLabel, targetLabel, targetNameLabel, fxChainLabel;
    juce::ComboBox insertBox;
    juce::ComboBox targetBox;
    juce::TextEditor targetNameBox;
    juce::Label fxChainNameLabel;
    juce::TextButton copyPlanBtn;
    juce::TextEditor planView;
    juce::Label insertsHint;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> insertAttach;

    bool ignoreCombo = false;
    int statusTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterEditor)
};
