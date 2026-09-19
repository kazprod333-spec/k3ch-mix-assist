#include "PluginEditor.h"

namespace
{
void colourLabel (juce::Label& l, juce::Colour c)
{
    l.setColour (juce::Label::textColourId, c);
}

void sectionTitle (juce::Label& l, const juce::String& text, juce::Colour gold)
{
    l.setText (text, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, gold);
    l.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
}

int stackRows (juce::Rectangle<int>& r, juce::Component& c, int h = 26, int gap = 4)
{
    c.setBounds (r.removeFromTop (h));
    r.removeFromTop (gap);
    return h + gap;
}
} // namespace

MasterEditor::MasterEditor (MasterProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      meter (p)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (980, 720, 1700, 1300);

    insertsModeBtn.setButtonText (juce::String::fromUTF8 ("Inserts / Mix"));
    masterModeBtn.setButtonText ("Master");
    insertsModeBtn.setClickingTogglesState (true);
    masterModeBtn.setClickingTogglesState (true);
    insertsModeBtn.setRadioGroupId (701);
    masterModeBtn.setRadioGroupId (701);
    insertsModeBtn.onClick = [this]
    {
        processor.setModeMaster (false);
        refreshMode();
        statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
        statusTicks = 28;
    };
    masterModeBtn.onClick = [this]
    {
        processor.setModeMaster (true);
        refreshMode();
        statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
        statusTicks = 28;
    };
    addAndMakeVisible (insertsModeBtn);
    addAndMakeVisible (masterModeBtn);

    bypassButton.setClickingTogglesState (true);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "bypass", bypassButton);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (meter);

    aboutButton.setButtonText (juce::String::fromUTF8 ("\xc3\x80 propos"));
    aboutButton.onClick = [this] { showAbout(); };
    addAndMakeVisible (aboutButton);

    constraintNote.setText (
        juce::String::fromUTF8 (
            "Un VST3 ne peut pas charger de plugins sur d\xe2\x80\x99" "autres canaux Console. "
            "Les cha\xc3\xae" "nes insert s\xe2\x80\x99" "appliquent via FX Chains / macros Studio Pro, "
            "pas via ce plug-in."),
        juce::dontSendNotification);
    colourLabel (constraintNote, lnf.palette.gold);
    constraintNote.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (constraintNote);

    colourLabel (statusLabel, lnf.palette.muted);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    addAndMakeVisible (pageMaster);
    addAndMakeVisible (pageInserts);

    presetLabel.setText (juce::String::fromUTF8 ("Preset master"), juce::dontSendNotification);
    colourLabel (presetLabel, lnf.palette.gold);
    pageMaster.addAndMakeVisible (presetLabel);

    int pi = 1;
    for (const auto& name : [&]
         {
             juce::StringArray n;
             for (const auto& pr : k3ch_master::allMasterPresets())
                 n.add (pr.name);
             return n;
         }())
        masterPresetBox.addItem (name, pi++);
    masterPresetBox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        applySelectedMasterPreset();
    };
    pageMaster.addAndMakeVisible (masterPresetBox);

    applyPresetBtn.setButtonText (juce::String::fromUTF8 ("Appliquer"));
    applyPresetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    applyPresetBtn.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    applyPresetBtn.onClick = [this] { applySelectedMasterPreset(); };
    pageMaster.addAndMakeVisible (applyPresetBtn);

    masterTabs.setOutline (0);
    masterTabs.setTabBarDepth (34);
    masterTabs.setColour (juce::TabbedComponent::backgroundColourId, lnf.palette.bg);
    masterTabs.setColour (juce::TabbedComponent::outlineColourId, lnf.palette.border);
    pageMaster.addAndMakeVisible (masterTabs);

    pageEq.lay = [this]
    {
        auto r = pageEq.getLocalBounds().reduced (16, 14);
        auto gains = r.removeFromTop (64);
        auto gL = gains.removeFromLeft ((gains.getWidth() - 16) / 2);
        gains.removeFromLeft (16);
        inputDb.setBounds (gL.removeFromTop (28));
        outputDb.setBounds (gains.removeFromTop (28));
        r.removeFromTop (10);
        const int colW = (r.getWidth() - 24) / 3;
        auto low = r.removeFromLeft (colW);
        r.removeFromLeft (12);
        auto mid = r.removeFromLeft (colW);
        r.removeFromLeft (12);
        auto high = r;
        eqLowTitle.setBounds (low.removeFromTop (22));
        low.removeFromTop (8);
        stackRows (low, lowHz);
        stackRows (low, lowG);
        eqMidTitle.setBounds (mid.removeFromTop (22));
        mid.removeFromTop (8);
        stackRows (mid, midOn, 30);
        stackRows (mid, midHz);
        stackRows (mid, midG);
        stackRows (mid, midQ);
        eqHighTitle.setBounds (high.removeFromTop (22));
        high.removeFromTop (8);
        stackRows (high, highHz);
        stackRows (high, highG);
    };
    pageSat.lay = [this]
    {
        auto r = pageSat.getLocalBounds().reduced (20, 18);
        auto left = r.removeFromLeft ((r.getWidth() - 20) / 2);
        r.removeFromLeft (20);
        stackRows (left, satOn, 32);
        stackRows (left, satDrv, 28);
        stackRows (left, satMix, 28);
        stackRows (r, width, 28);
        stackRows (r, bassMono, 28);
    };
    pageDyn.lay = [this]
    {
        auto r = pageDyn.getLocalBounds().reduced (16, 14);
        grMeter.setBounds (r.removeFromTop (32));
        r.removeFromTop (12);
        auto c1 = r.removeFromLeft ((r.getWidth() - 24) / 3);
        r.removeFromLeft (12);
        auto c2 = r.removeFromLeft ((r.getWidth() - 12) / 2);
        r.removeFromLeft (12);
        auto c3 = r;
        stackRows (c1, clipOn, 30);
        stackRows (c1, clipCeil);
        stackRows (c2, limOn, 30);
        stackRows (c2, limCeil);
        stackRows (c2, limRel);
        stackRows (c2, limLook);
        stackRows (c3, tpOn, 30);
        stackRows (c3, tpCeil);
    };

    sectionTitle (eqLowTitle, juce::String::fromUTF8 ("Grave  \xc2\xb7  shelf"), lnf.palette.gold);
    sectionTitle (eqMidTitle, juce::String::fromUTF8 ("M\xc3\xa9" "dium  \xc2\xb7  cloche"), lnf.palette.gold);
    sectionTitle (eqHighTitle, juce::String::fromUTF8 ("Aigu  \xc2\xb7  shelf"), lnf.palette.gold);
    inputDb.setup (processor.apvts, "input_db", juce::String::fromUTF8 ("Entr\xc3\xa9" "e"), " dB");
    outputDb.setup (processor.apvts, "output_db", "Sortie", " dB");
    lowHz.setup (processor.apvts, "eq_low_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    lowG.setup (processor.apvts, "eq_low_gain", "Gain", " dB");
    highHz.setup (processor.apvts, "eq_high_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    highG.setup (processor.apvts, "eq_high_gain", "Gain", " dB");
    midOn.setup (processor.apvts, "eq_mid_on", juce::String::fromUTF8 ("Bande m\xc3\xa9" "dium"));
    midHz.setup (processor.apvts, "eq_mid_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    midG.setup (processor.apvts, "eq_mid_gain", "Gain", " dB");
    midQ.setup (processor.apvts, "eq_mid_q", "Facteur Q", {});
    for (auto* c : { &eqLowTitle, &eqMidTitle, &eqHighTitle })
        addTo (pageEq, *c);
    for (auto* c : { &inputDb, &outputDb, &lowHz, &lowG, &highHz, &highG, &midHz, &midG, &midQ })
        addTo (pageEq, *c);
    addTo (pageEq, midOn);

    satOn.setup (processor.apvts, "sat_on", "Saturation / glue");
    satDrv.setupPercent (processor.apvts, "sat_drive", "Drive");
    satMix.setupPercent (processor.apvts, "sat_mix", "Mix");
    width.setup (processor.apvts, "width", juce::String::fromUTF8 ("Largeur"), {});
    bassMono.setup (processor.apvts, "bass_mono_hz", "Basse mono", " Hz");
    addTo (pageSat, satOn);
    for (auto* c : { &satDrv, &satMix, &width, &bassMono })
        addTo (pageSat, *c);

    clipOn.setup (processor.apvts, "clip_on", "Soft clip");
    limOn.setup (processor.apvts, "lim_on", "Limiteur");
    tpOn.setup (processor.apvts, "tp_on", "True peak");
    clipCeil.setup (processor.apvts, "clip_ceiling", "Plafond clip", " dB");
    limCeil.setup (processor.apvts, "lim_ceiling", "Plafond lim.", " dB");
    {
        juce::String relache ("Rel");
        relache += juce::String::charToString (juce::juce_wchar (0x00E2));
        relache += "chement";
        limRel.setup (processor.apvts, "lim_release", relache, " ms");
    }
    limLook.setup (processor.apvts, "lim_lookahead", "Lookahead", " ms");
    tpCeil.setup (processor.apvts, "tp_ceiling", "Plafond TP", " dB");
    addTo (pageDyn, grMeter);
    addTo (pageDyn, clipOn);
    addTo (pageDyn, limOn);
    addTo (pageDyn, tpOn);
    for (auto* c : { &clipCeil, &limCeil, &limRel, &limLook, &tpCeil })
        addTo (pageDyn, *c);

    masterTabs.addTab ("EQ / Gains", lnf.palette.bgCard, &pageEq, false);
    masterTabs.addTab ("Sat / Largeur", lnf.palette.bgCard, &pageSat, false);
    masterTabs.addTab (juce::String::fromUTF8 ("Dynamique"), lnf.palette.bgCard, &pageDyn, false);

    insertListLabel.setText (juce::String::fromUTF8 ("Preset insert / mix"), juce::dontSendNotification);
    colourLabel (insertListLabel, lnf.palette.gold);
    pageInserts.addAndMakeVisible (insertListLabel);

    insertBox.setTextWhenNothingSelected (juce::String::fromUTF8 ("Cha\xc3\xae" "ne"));
    insertAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, "insert_preset", insertBox);
    insertBox.onChange = [this]
    {
        updatePlanView();
        if (const auto* pr = processor.insertLibrary.at (insertBox.getSelectedItemIndex()))
            fxChainNameLabel.setText (juce::String::fromUTF8 ("FX Chain S1 : ") + pr->fxChainName,
                                      juce::dontSendNotification);
    };
    pageInserts.addAndMakeVisible (insertBox);

    targetLabel.setText (juce::String::fromUTF8 ("Canal Console #"), juce::dontSendNotification);
    colourLabel (targetLabel, lnf.palette.gold);
    pageInserts.addAndMakeVisible (targetLabel);

    for (int i = 1; i <= 125; ++i)
        targetBox.addItem (juce::String (i), i);
    targetBox.setSelectedId (processor.currentTargetChannel(), juce::dontSendNotification);
    targetBox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        if (auto* param = dynamic_cast<juce::AudioParameterInt*> (processor.apvts.getParameter ("target_channel")))
            param->setValueNotifyingHost (param->convertTo0to1 ((float) targetBox.getSelectedId()));
        updatePlanView();
    };
    pageInserts.addAndMakeVisible (targetBox);

    targetNameLabel.setText (juce::String::fromUTF8 ("Nom du canal (m\xc3\xa9" "mo)"), juce::dontSendNotification);
    colourLabel (targetNameLabel, lnf.palette.gold);
    pageInserts.addAndMakeVisible (targetNameLabel);

    targetNameBox.setTextToShowWhenEmpty (juce::String::fromUTF8 ("ex. Voix Lead, Bus_VOX, 808"),
                                          lnf.palette.muted);
    targetNameBox.setText (processor.targetChannelName);
    targetNameBox.onTextChange = [this]
    {
        processor.targetChannelName = targetNameBox.getText();
        updatePlanView();
    };
    pageInserts.addAndMakeVisible (targetNameBox);

    colourLabel (fxChainNameLabel, lnf.palette.teal);
    pageInserts.addAndMakeVisible (fxChainNameLabel);

    copyPlanBtn.setButtonText (juce::String::fromUTF8 ("Copier le plan"));
    copyPlanBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    copyPlanBtn.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    copyPlanBtn.onClick = [this] { copyPlan(); };
    pageInserts.addAndMakeVisible (copyPlanBtn);

    insertsHint.setText (
        juce::String::fromUTF8 (
            "Pose K3CH Presets sur ce canal pour la cha\xc3\xae" "ne live. "
            "Ce Master ne pilote pas les autres slots. "
            "Enregistre la FX Chain une fois, puis rappelle-la (Navigateur ou Macro Organizer)."),
        juce::dontSendNotification);
    colourLabel (insertsHint, lnf.palette.muted);
    insertsHint.setJustificationType (juce::Justification::topLeft);
    pageInserts.addAndMakeVisible (insertsHint);

    styleReadOnly (planView);
    pageInserts.addAndMakeVisible (planView);

    pageMaster.setVisible (false);
    pageInserts.setVisible (true);

    setSize (1180, 820);

    ignoreCombo = true;
    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter ("master_preset")))
        masterPresetBox.setSelectedItemIndex (ch->getIndex(), juce::dontSendNotification);
    ignoreCombo = false;

    refreshMode();
    updatePlanView();
    statusLabel.setText (processor.lastStatus.isNotEmpty()
                             ? processor.lastStatus
                             : juce::String::fromUTF8 (
                                   "Studio Pro 8 \xe2\x80\x94 Master DSP ou plans FX Chain. Pas de remote insert."),
                         juce::dontSendNotification);
    startTimerHz (8);
}

MasterEditor::~MasterEditor()
{
    stopTimer();
    bypassAttachment.reset();
    insertAttach.reset();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void MasterEditor::addTo (juce::Component& page, juce::Component& child)
{
    page.addAndMakeVisible (child);
}

void MasterEditor::styleReadOnly (juce::TextEditor& editor)
{
    editor.setMultiLine (true, true);
    editor.setReadOnly (true);
    editor.setScrollbarsShown (true);
    editor.setCaretVisible (false);
    editor.setPopupMenuEnabled (true);
    editor.setFont (juce::Font (juce::FontOptions (13.0f)));
    editor.setIndents (8, 6);
    editor.setColour (juce::TextEditor::backgroundColourId, lnf.palette.bg);
    editor.setColour (juce::TextEditor::outlineColourId, lnf.palette.border);
    editor.setColour (juce::TextEditor::focusedOutlineColourId, lnf.palette.border);
    editor.setColour (juce::TextEditor::textColourId, lnf.palette.text);
}

void MasterEditor::refreshMode()
{
    const bool master = processor.isMasterMode();
    ignoreCombo = true;
    masterModeBtn.setToggleState (master, juce::dontSendNotification);
    insertsModeBtn.setToggleState (! master, juce::dontSendNotification);
    ignoreCombo = false;
    pageMaster.setVisible (master);
    pageInserts.setVisible (! master);
    resized();
    repaint();
}

void MasterEditor::timerCallback()
{
    if (masterModeBtn.getToggleState() != processor.isMasterMode())
        refreshMode();

    ignoreCombo = true;
    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter ("master_preset")))
        if (masterPresetBox.getSelectedItemIndex() != ch->getIndex())
            masterPresetBox.setSelectedItemIndex (ch->getIndex(), juce::dontSendNotification);
    if (targetBox.getSelectedId() != processor.currentTargetChannel())
        targetBox.setSelectedId (processor.currentTargetChannel(), juce::dontSendNotification);
    ignoreCombo = false;

    grMeter.setGrDb (processor.getLimiterGrDb());
    if (statusTicks > 0 && --statusTicks == 0)
        statusLabel.setText (processor.isMasterMode()
                                 ? juce::String::fromUTF8 ("Mode Master \xe2\x80\x94 DSP sur ce bus")
                                 : juce::String::fromUTF8 (
                                       "Mode Inserts \xe2\x80\x94 plans FX Chain, audio en pass-through"),
                             juce::dontSendNotification);
}

void MasterEditor::applySelectedMasterPreset()
{
    processor.loadMasterPreset (masterPresetBox.getSelectedItemIndex(), true);
    statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
    statusTicks = 28;
}

void MasterEditor::updatePlanView()
{
    const auto text = processor.currentPlanText();
    if (planView.getText() != text)
        planView.setText (text, false);
    if (const auto* pr = processor.insertLibrary.at (processor.currentInsertIndex()))
        fxChainNameLabel.setText (juce::String::fromUTF8 ("FX Chain S1 : ") + pr->fxChainName,
                                  juce::dontSendNotification);
}

void MasterEditor::copyPlan()
{
    juce::SystemClipboard::copyTextToClipboard (processor.currentPlanText());
    statusLabel.setText (juce::String::fromUTF8 ("Plan FX Chain copi\xc3\xa9 dans le presse-papiers"),
                         juce::dontSendNotification);
    statusTicks = 24;
}

void MasterEditor::showAbout()
{
    juce::String t;
    t += "K3CH Master " + juce::String (K3CH_VERSION) + "\n";
    t += processor.insertLibrary.getStudio() + juce::String::fromUTF8 (" \xe2\x80\x94 Alger\n");
    t += processor.insertLibrary.getHost() + "\n\n";
    t += juce::String::fromUTF8 (
        "Mode Master : cha\xc3\xae" "ne DSP (gain, EQ, sat, largeur, clip, limiteur, true-peak) "
        "sur le bus o\xc3\xb9 ce plug-in est pos\xc3\xa9.\n\n"
        "Mode Inserts / Mix : biblioth\xc3\xa8que de plans. "
        "Studio Pro charge les FX Chains via le Navigateur ou Macro Organizer. "
        "Ce VST3 ne peut pas ins\xc3\xa9rer de plugins sur d\xe2\x80\x99" "autres canaux.\n\n"
        "Voir StudioOne/README.md dans le d\xc3\xa9p\xc3\xb4t.\n");
    if (processor.insertLibrary.getVersion().isNotEmpty())
        t += "Plans : " + processor.insertLibrary.getVersion() + "\n";

    juce::AlertWindow::showMessageBoxAsync (
        juce::MessageBoxIconType::InfoIcon,
        JucePlugin_Name,
        t);
}

void MasterEditor::paint (juce::Graphics& g)
{
    const auto& pal = lnf.palette;
    g.fillAll (pal.bg);

    auto header = getLocalBounds().removeFromTop (78);
    g.setColour (pal.bgPanel);
    g.fillRect (header);
    g.setColour (pal.gold);
    g.fillRect (0, 78, getWidth(), 2);

    g.setColour (pal.gold);
    g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
    g.drawText ("K3CH", juce::Rectangle<int> (20, 14, 70, 18), juce::Justification::centredLeft);

    g.setColour (pal.text);
    g.setFont (juce::Font (juce::FontOptions (22.0f).withStyle ("Bold")));
    g.drawText ("MASTER", juce::Rectangle<int> (20, 30, 280, 26), juce::Justification::centredLeft);

    g.setColour (pal.muted);
    g.setFont (juce::Font (juce::FontOptions (12.5f)));
    g.drawText (processor.insertLibrary.getStudio() + juce::String::fromUTF8 ("  \xc2\xb7  Alger  \xc2\xb7  Studio Pro 8"),
                juce::Rectangle<int> (20, 54, 520, 18), juce::Justification::centredLeft);

    auto footer = getLocalBounds().removeFromBottom (48);
    g.setColour (pal.bgPanel);
    g.fillRect (footer);
    g.setColour (pal.border);
    g.fillRect (footer.getX(), footer.getY(), footer.getWidth(), 1);
}

void MasterEditor::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop (80);
    auto footer = bounds.removeFromBottom (48);

    auto headerRight = header.removeFromRight (520).reduced (12, 14);
    bypassButton.setBounds (headerRight.removeFromRight (88).reduced (2, 8));
    headerRight.removeFromRight (8);
    meter.setBounds (headerRight.removeFromRight (210).reduced (2, 0));
    headerRight.removeFromRight (10);
    masterModeBtn.setBounds (headerRight.removeFromRight (100).reduced (2, 6));
    headerRight.removeFromRight (6);
    insertsModeBtn.setBounds (headerRight.removeFromRight (120).reduced (2, 6));

    auto footInner = footer.reduced (16, 8);
    aboutButton.setBounds (footInner.removeFromRight (100));
    footInner.removeFromRight (10);
    statusLabel.setBounds (footInner);

    constraintNote.setBounds (bounds.removeFromTop (26).reduced (20, 3));

    auto body = bounds.reduced (12, 8);
    pageMaster.setBounds (body);
    pageInserts.setBounds (body);

    {
        auto r = pageMaster.getLocalBounds();
        auto strip = r.removeFromTop (40);
        presetLabel.setBounds (strip.removeFromLeft (120));
        applyPresetBtn.setBounds (strip.removeFromRight (120));
        strip.removeFromRight (8);
        masterPresetBox.setBounds (strip);
        r.removeFromTop (8);
        masterTabs.setBounds (r);
    }

    {
        auto r = pageInserts.getLocalBounds();
        insertsHint.setBounds (r.removeFromTop (40));
        r.removeFromTop (8);
        auto strip = r.removeFromTop (32);
        insertListLabel.setBounds (strip.removeFromLeft (150));
        insertBox.setBounds (strip.removeFromLeft (200));
        strip.removeFromLeft (12);
        targetLabel.setBounds (strip.removeFromLeft (120));
        targetBox.setBounds (strip.removeFromLeft (80));
        strip.removeFromLeft (12);
        targetNameLabel.setBounds (strip.removeFromLeft (140));
        targetNameBox.setBounds (strip);
        r.removeFromTop (10);
        auto row = r.removeFromTop (28);
        copyPlanBtn.setBounds (row.removeFromRight (150));
        row.removeFromRight (8);
        fxChainNameLabel.setBounds (row);
        r.removeFromTop (8);
        planView.setBounds (r);
    }
}
