#pragma once

#include "PluginProcessor.h"
#include "UI/ControlRow.h"
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
    void addTo (juce::Component& page, juce::Component& child);

    void layoutChainPage();
    void layoutEqPage();
    void layoutDynPage();
    void layoutSatPage();
    void layoutPlanPage();

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

    juce::Label sendTargetLabel;
    juce::ComboBox sendTargetBox;
    juce::Label fxPresetLabel;
    juce::ComboBox fxPresetBox;
    juce::TextButton placeButton { "Placer" };

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    LayoutPage pageChain, pageEq, pageDyn, pageSat, pageSendA, pageSendB, pagePlan;

    juce::Label chainHint;
    juce::TextEditor chainView;
    ToggleRow hpfOn;
    ChoiceRow hpfSlope;
    ControlRow hpfHz, inputDb, dryWet, returnMix, outputDb;

    juce::Label eqMudTitle, eqMidTitle, eqAirTitle;
    ControlRow eqLowHz, eqLowG, eqLowQ;
    ControlRow eqMidHz, eqMidG, eqMidQ;
    ControlRow eqAirHz, eqAirG;

    juce::Label compTitle, deessTitle;
    ControlRow compTh, compRatio, compAtk, compRel, compMk;
    GrMeter grMeter;
    ControlRow deessHz, deessAmt;

    ToggleRow satOn;
    ControlRow satDrv, satMix;

    juce::Label sendATitle, sendBTitle;
    juce::ComboBox sendABox, sendBBox;
    ControlRow sendALevel, sendBLevel;
    juce::Label sendANotes, sendBNotes;
    ControlRow saHpf, saLpf, saSmash, saDsHz, saDs, saSatDrv, saSatMix;
    ControlRow saRevMix, saRevDec, saRevDamp, saDlyMix, saDlyMs, saDlyFb, saHarshHz, saHarshCut;
    ControlRow sbHpf, sbLpf, sbSmash, sbDsHz, sbDs, sbSatDrv, sbSatMix;
    ControlRow sbRevMix, sbRevDec, sbRevDamp, sbDlyMix, sbDlyMs, sbDlyFb, sbHarshHz, sbHarshCut;

    juce::Label planHint;
    juce::TextButton copyPlanButton;
    juce::TextEditor planView;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> sendABoxAttach, sendBBoxAttach;

    bool ignoreCombo = false;
    int statusTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsEditor)
};
