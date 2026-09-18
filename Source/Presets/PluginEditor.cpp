#include "PluginEditor.h"

PresetsEditor::PresetsEditor (PresetsProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      meter (p)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (1100, 760, 1800, 1400);
    setSize (1260, 920);

    applyChainButton.setButtonText (juce::String::fromUTF8 ("Appliquer la cha\xc3\xae" "ne"));
    copyPlanButton.setButtonText (juce::String::fromUTF8 ("Copier"));

    commandBox.setTextToShowWhenEmpty (
        juce::String::fromUTF8 ("optionnel : charge ModernRap  \xc2\xb7  mets Air sur send A"),
        lnf.palette.muted);
    commandBox.setJustification (juce::Justification::centredLeft);
    commandBox.setIndents (10, 0);
    commandBox.setEscapeAndReturnKeysConsumed (true);
    commandBox.onReturnKey = [this] { applyCommand(); };
    addAndMakeVisible (commandBox);

    applyButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    applyButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    applyButton.onClick = [this] { applyCommand(); };
    addAndMakeVisible (applyButton);

    constraintNote.setText (
        juce::String::fromUTF8 (
            "Un VST3 ne peut pas ins\xc3\xa9rer ni contr\xc3\xb4ler d\xe2\x80\x99" "autres plugins tiers "
            "sur les inserts du mixer FL."),
        juce::dontSendNotification);
    constraintNote.setColour (juce::Label::textColourId, lnf.palette.gold);
    constraintNote.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (constraintNote);

    statusLabel.setColour (juce::Label::textColourId, lnf.palette.muted);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    bypassButton.setClickingTogglesState (true);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "bypass", bypassButton);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (meter);

    aboutButton.setButtonText (juce::String::fromUTF8 ("\xc3\x80 propos"));
    aboutButton.onClick = [this] { showAbout(); };
    addAndMakeVisible (aboutButton);

    chainLabel.setText (juce::String::fromUTF8 ("Cha\xc3\xae" "ne (insert FL)"), juce::dontSendNotification);
    chainLabel.setColour (juce::Label::textColourId, lnf.palette.gold);
    addAndMakeVisible (chainLabel);

    vocalBox.setTextWhenNothingSelected (juce::String::fromUTF8 ("Cha\xc3\xae" "ne"));
    int i = 1;
    for (const auto& name : processor.library.vocalNames())
        vocalBox.addItem (name, i++);
    vocalBox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        applySelectedChain();
    };
    addAndMakeVisible (vocalBox);

    applyChainButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    applyChainButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    applyChainButton.onClick = [this] { applySelectedChain(); };
    addAndMakeVisible (applyChainButton);

    chainHint.setText (juce::String::fromUTF8 (
                           "Pose CE plug-in sur l\xe2\x80\x99insert FL choisi. "
                           "La cha\xc3\xae" "ne s\xe2\x80\x99" "applique ici, dans l\xe2\x80\x99ordre, avec les r\xc3\xa9" "glages."),
                       juce::dontSendNotification);
    chainHint.setColour (juce::Label::textColourId, lnf.palette.muted);
    chainHint.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (chainHint);

    styleReadOnly (chainView);
    addAndMakeVisible (chainView);

    planHint.setText (juce::String::fromUTF8 (
                          "Checklist manuelle si tu veux recr\xc3\xa9" "er CLA-76, RVox, etc. sur le mixer FL."),
                      juce::dontSendNotification);
    planHint.setColour (juce::Label::textColourId, lnf.palette.muted);
    planHint.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (planHint);

    copyPlanButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    copyPlanButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    copyPlanButton.onClick = [this] { copyFlInsertPlan(); };
    addAndMakeVisible (copyPlanButton);

    styleReadOnly (planView);
    addAndMakeVisible (planView);

    sendTargetLabel.setText (juce::String::fromUTF8 ("1. Placer sur"), juce::dontSendNotification);
    sendTargetLabel.setColour (juce::Label::textColourId, lnf.palette.gold);
    addAndMakeVisible (sendTargetLabel);

    sendTargetBox.addItem ("Send A", 1);
    sendTargetBox.addItem ("Send B", 2);
    sendTargetBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (sendTargetBox);

    fxPresetLabel.setText ("2. Preset FX", juce::dontSendNotification);
    fxPresetLabel.setColour (juce::Label::textColourId, lnf.palette.gold);
    addAndMakeVisible (fxPresetLabel);

    fxPresetBox.setTextWhenNothingSelected ("Preset FX");
    int fxId = 1;
    for (const auto& name : processor.library.fxNames())
        fxPresetBox.addItem (name, fxId++);
    const int air = processor.library.findFxIndex ("air");
    fxPresetBox.setSelectedItemIndex (air >= 0 ? air : 0, juce::dontSendNotification);
    fxPresetBox.onChange = [this]
    {
        if (const auto* fx = processor.library.fxAt (fxPresetBox.getSelectedItemIndex()))
            fxChoiceReadout.setText (fx->notes, juce::dontSendNotification);
    };
    addAndMakeVisible (fxPresetBox);

    placeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    placeButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    placeButton.onClick = [this] { placeFxOnSelectedSend(); };
    addAndMakeVisible (placeButton);

    placeHint.setText (juce::String::fromUTF8 (
                           "Je choisis le send, je choisis le preset, j'applique. "
                           "Sends internes \xc3\xa0 CE plug-in \xe2\x80\x94 pas le mixer FL."),
                       juce::dontSendNotification);
    placeHint.setColour (juce::Label::textColourId, lnf.palette.muted);
    placeHint.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (placeHint);

    fxChoiceReadout.setColour (juce::Label::textColourId, lnf.palette.muted);
    fxChoiceReadout.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (fxChoiceReadout);

    auto setupName = [this] (juce::Label& l)
    {
        l.setColour (juce::Label::textColourId, lnf.palette.text);
        l.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
        l.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (l);
    };
    setupName (sendAName);
    setupName (sendBName);

    auto setupLevel = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 62, 20);
        s.setTextValueSuffix (" dB");
        addAndMakeVisible (s);
    };
    setupLevel (sendALevel);
    setupLevel (sendBLevel);
    sendAAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "send_a_db", sendALevel);
    sendBAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "send_b_db", sendBLevel);

    sendAReadout.setColour (juce::Label::textColourId, lnf.palette.muted);
    sendBReadout.setColour (juce::Label::textColourId, lnf.palette.muted);
    sendAReadout.setJustificationType (juce::Justification::topLeft);
    sendBReadout.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (sendAReadout);
    addAndMakeVisible (sendBReadout);

    auto setupMix = [this] (juce::Slider& s, const juce::String& suffix)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 20);
        s.setTextValueSuffix (suffix);
        addAndMakeVisible (s);
    };
    setupMix (dryWet, " %");
    setupMix (returnMix, " %");
    setupMix (outputDb, " dB");

    dryAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "dry_wet", dryWet);
    retAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "return_mix", returnMix);
    outAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "output_db", outputDb);

    dryWet.textFromValueFunction = [] (double v)
    {
        return juce::String (juce::roundToInt (v * 100.0));
    };
    dryWet.valueFromTextFunction = [] (const juce::String& t)
    {
        return t.getDoubleValue() / 100.0;
    };
    returnMix.textFromValueFunction = [] (double v)
    {
        return juce::String (juce::roundToInt (v * 100.0));
    };
    returnMix.valueFromTextFunction = [] (const juce::String& t)
    {
        return t.getDoubleValue() / 100.0;
    };
    dryWet.updateText();
    returnMix.updateText();

    dryLabel.setText ("Dry/Wet", juce::dontSendNotification);
    returnLabel.setText ("Return mix", juce::dontSendNotification);
    outLabel.setText ("Output", juce::dontSendNotification);
    for (auto* l : { &dryLabel, &returnLabel, &outLabel })
    {
        l->setColour (juce::Label::textColourId, lnf.palette.muted);
        addAndMakeVisible (l);
    }

    syncCombosFromProcessor();
    updateReadouts();
    if (const auto* fx = processor.library.fxAt (fxPresetBox.getSelectedItemIndex()))
        fxChoiceReadout.setText (fx->notes, juce::dontSendNotification);

    statusLabel.setText (processor.lastStatus.isNotEmpty()
                             ? processor.lastStatus
                             : juce::String::fromUTF8 (
                                   "Cha\xc3\xae" "ne insert + sends internes \xe2\x80\x94 pas le mixer FL Studio"),
                         juce::dontSendNotification);
    startTimerHz (8);
}

PresetsEditor::~PresetsEditor()
{
    stopTimer();
    bypassAttachment.reset();
    sendAAttach.reset();
    sendBAttach.reset();
    dryAttach.reset();
    retAttach.reset();
    outAttach.reset();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void PresetsEditor::styleReadOnly (juce::TextEditor& editor)
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

void PresetsEditor::timerCallback()
{
    syncCombosFromProcessor();
    updateReadouts();
    if (statusTicks > 0 && --statusTicks == 0)
        statusLabel.setText (juce::String::fromUTF8 (
                                 "Cha\xc3\xae" "ne insert + sends internes \xe2\x80\x94 pas le mixer FL Studio"),
                             juce::dontSendNotification);
}

void PresetsEditor::syncCombosFromProcessor()
{
    auto choiceIndex = [this] (const char* id) -> int
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (id)))
            return p->getIndex();
        return 0;
    };

    ignoreCombo = true;
    vocalBox.setSelectedItemIndex (choiceIndex ("vocal_preset"), juce::dontSendNotification);
    ignoreCombo = false;
}

void PresetsEditor::updateReadouts()
{
    auto liveFxName = [this] (const char* paramId) -> juce::String
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId)))
            idx = p->getIndex();
        if (const auto* fx = processor.library.fxAt (idx))
            return fx->name;
        return "Off";
    };

    if (const auto* p = processor.library.vocalAt (vocalBox.getSelectedItemIndex()))
    {
        juce::String chain;
        for (const auto& step : p->appliedChain)
        {
            auto settings = step.settings;
            if (step.title.containsIgnoreCase ("Send A"))
                settings = liveFxName ("send_a_preset");
            else if (step.title.containsIgnoreCase ("Send B"))
                settings = liveFxName ("send_b_preset");
            chain += juce::String (step.n) + ".  " + step.title;
            if (settings.isNotEmpty())
                chain += juce::String::fromUTF8 ("  \xe2\x80\x94  ") + settings;
            chain += "\n";
        }
        chain = chain.trimEnd();
        if (chainView.getText() != chain)
            chainView.setText (chain, false);
        const auto plan = p->flInsertPlanText();
        if (planView.getText() != plan)
            planView.setText (plan, false);
    }

    auto fxNotes = [this] (const char* paramId) -> juce::String
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId)))
            idx = p->getIndex();
        if (const auto* fx = processor.library.fxAt (idx))
            return fx->notes;
        return {};
    };

    sendAName.setText (liveFxName ("send_a_preset"), juce::dontSendNotification);
    sendBName.setText (liveFxName ("send_b_preset"), juce::dontSendNotification);
    sendAReadout.setText (fxNotes ("send_a_preset"), juce::dontSendNotification);
    sendBReadout.setText (fxNotes ("send_b_preset"), juce::dontSendNotification);
}

void PresetsEditor::applySelectedChain()
{
    processor.loadVocalIndex (vocalBox.getSelectedItemIndex(), true);
    updateReadouts();
    statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
    statusTicks = 28;
}

void PresetsEditor::placeFxOnSelectedSend()
{
    const int sendSlot = juce::jlimit (0, 1, sendTargetBox.getSelectedItemIndex());
    const int fxIndex = fxPresetBox.getSelectedItemIndex();
    processor.loadFxIndex (sendSlot, fxIndex, true, true);
    updateReadouts();
    statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
    statusTicks = 24;
}

void PresetsEditor::copyFlInsertPlan()
{
    if (const auto* p = processor.library.vocalAt (vocalBox.getSelectedItemIndex()))
    {
        juce::SystemClipboard::copyTextToClipboard (p->flInsertPlanText());
        statusLabel.setText (juce::String::fromUTF8 ("Plan d\xe2\x80\x99inserts copi\xc3\xa9 dans le presse-papiers"),
                             juce::dontSendNotification);
        statusTicks = 24;
    }
}

void PresetsEditor::applyCommand()
{
    const auto cmd = processor.applyCommand (commandBox.getText());
    statusLabel.setText (cmd.message, juce::dontSendNotification);
    statusTicks = 24;
    syncCombosFromProcessor();
    updateReadouts();
    if (cmd.ok)
        commandBox.clear();
}

void PresetsEditor::showAbout()
{
    juce::String t;
    t += "K3CH Presets " + juce::String (K3CH_VERSION) + "\n";
    t += processor.library.getStudio() + juce::String::fromUTF8 (" \xe2\x80\x94 Alger\n\n");
    t += juce::String::fromUTF8 (
        "Cha\xc3\xae" "ne (insert FL) = tout le traitement interne, dans l\xe2\x80\x99ordre, "
        "avec les r\xc3\xa9" "glages du preset. Pose K3CH Presets sur l\xe2\x80\x99insert FL de ton choix.\n"
        "Un VST3 ne peut pas ins\xc3\xa9rer/contr\xc3\xb4ler CLA-76, RVox, etc. sur le mixer "
        "\xe2\x80\x94 voir Plan d\xe2\x80\x99inserts FL (manuel).\n"
        "Sends A/B = FX internes (Placer sur).\n\n");
    if (processor.library.getVersion().isNotEmpty())
        t += "Presets runtime : " + processor.library.getVersion() + "\n";
    t += juce::String::fromUTF8 ("D\xc3\xa9riv\xc3\xa9 de mix-knowledge.json. Local only.");

    juce::AlertWindow::showMessageBoxAsync (
        juce::MessageBoxIconType::InfoIcon,
        JucePlugin_Name,
        t);
}

void PresetsEditor::paint (juce::Graphics& g)
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
    g.drawText ("PRESETS", juce::Rectangle<int> (20, 30, 280, 26), juce::Justification::centredLeft);

    g.setColour (pal.muted);
    g.setFont (juce::Font (juce::FontOptions (12.5f)));
    g.drawText (processor.library.getStudio() + juce::String::fromUTF8 ("  \xc2\xb7  Alger"),
                juce::Rectangle<int> (20, 54, 420, 18), juce::Justification::centredLeft);

    auto footer = getLocalBounds().removeFromBottom (48);
    g.setColour (pal.bgPanel);
    g.fillRect (footer);
    g.setColour (pal.border);
    g.fillRect (footer.getX(), footer.getY(), footer.getWidth(), 1);

    auto body = getLocalBounds();
    body.removeFromTop (80);
    body.removeFromBottom (48);
    auto cmdBar = body.removeFromTop (78);
    g.setColour (pal.bgSidebar);
    g.fillRect (cmdBar);
    g.setColour (pal.border);
    g.fillRect (cmdBar.getX(), cmdBar.getBottom() - 1, cmdBar.getWidth(), 1);

    g.setColour (pal.goldDim);
    g.setFont (juce::Font (juce::FontOptions (11.5f).withStyle ("Bold")));
    g.drawText ("COMMANDE (optionnel)", juce::Rectangle<int> (20, 88, 280, 16), juce::Justification::centredLeft);

    auto card = [&] (juce::Rectangle<int> r, const juce::String& title)
    {
        g.setColour (pal.bgCard);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (pal.gold);
        g.fillRect ((float) r.getX(), (float) r.getY() + 10.0f, 3.0f, 18.0f);
        g.setColour (pal.border.withAlpha (0.7f));
        g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);
        g.setColour (pal.gold);
        g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
        g.drawText (title, r.reduced (16, 10).removeFromTop (22), juce::Justification::centredLeft);
    };

    card (chainCard, juce::String::fromUTF8 ("CHA\xc3\x8eNE (INSERT FL)"));
    card (planCard, juce::String::fromUTF8 ("PLAN D\xe2\x80\x99INSERTS FL (MANUEL)"));
    card (placeCard, juce::String::fromUTF8 ("PRESET FX  \xc2\xb7  PLACER SUR UN SEND"));
    card (sendACard, juce::String::fromUTF8 ("SEND A  \xc2\xb7  actuel"));
    card (sendBCard, juce::String::fromUTF8 ("SEND B  \xc2\xb7  actuel"));
}

void PresetsEditor::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop (80);
    auto footer = bounds.removeFromBottom (48);

    auto headerRight = header.removeFromRight (380).reduced (16, 16);
    bypassButton.setBounds (headerRight.removeFromRight (90).reduced (4, 8));
    meter.setBounds (headerRight.reduced (4, 2));

    auto footInner = footer.reduced (16, 8);
    aboutButton.setBounds (footInner.removeFromRight (100));
    footInner.removeFromRight (10);
    statusLabel.setBounds (footInner);

    auto cmdBar = bounds.removeFromTop (78).reduced (16, 8);
    cmdBar.removeFromTop (16);
    auto cmdRow = cmdBar.removeFromTop (32);
    applyButton.setBounds (cmdRow.removeFromRight (120));
    cmdRow.removeFromRight (8);
    commandBox.setBounds (cmdRow);

    constraintNote.setBounds (bounds.removeFromTop (26).reduced (20, 2));

    auto cols = bounds.reduced (16, 10);
    auto top = cols.removeFromTop (juce::jmax (340, cols.getHeight() * 58 / 100));
    cols.removeFromTop (10);
    chainCard = top.removeFromLeft (juce::jmax (360, top.getWidth() * 42 / 100));
    top.removeFromLeft (12);
    auto rightCol = top;
    planCard = rightCol.removeFromTop (juce::jmax (170, (rightCol.getHeight() - 10) * 54 / 100));
    rightCol.removeFromTop (10);
    placeCard = rightCol;
    sendACard = cols.removeFromLeft ((cols.getWidth() - 12) / 2);
    cols.removeFromLeft (12);
    sendBCard = cols;

    auto inner = [] (juce::Rectangle<int> r)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        return r;
    };

    auto v = inner (chainCard);
    chainLabel.setBounds (v.removeFromTop (20));
    v.removeFromTop (4);
    auto chainRow = v.removeFromTop (32);
    applyChainButton.setBounds (chainRow.removeFromRight (168));
    chainRow.removeFromRight (8);
    vocalBox.setBounds (chainRow);
    v.removeFromTop (6);
    chainHint.setBounds (v.removeFromTop (36));
    v.removeFromTop (6);
    auto mixRow = v.removeFromTop (24);
    dryLabel.setBounds (mixRow.removeFromLeft (72));
    dryWet.setBounds (mixRow);
    v.removeFromTop (4);
    mixRow = v.removeFromTop (24);
    returnLabel.setBounds (mixRow.removeFromLeft (72));
    returnMix.setBounds (mixRow);
    v.removeFromTop (4);
    mixRow = v.removeFromTop (24);
    outLabel.setBounds (mixRow.removeFromLeft (72));
    outputDb.setBounds (mixRow);
    v.removeFromTop (8);
    chainView.setBounds (v);

    auto plan = inner (planCard);
    auto planTop = plan.removeFromTop (28);
    copyPlanButton.setBounds (planTop.removeFromRight (88));
    planTop.removeFromRight (8);
    planHint.setBounds (planTop);
    plan.removeFromTop (6);
    planView.setBounds (plan);

    auto p = inner (placeCard);
    auto destRow = p.removeFromTop (28);
    sendTargetLabel.setBounds (destRow.removeFromLeft (110));
    destRow.removeFromLeft (8);
    sendTargetBox.setBounds (destRow.removeFromLeft (juce::jmin (160, destRow.getWidth())));
    p.removeFromTop (6);
    auto fxRow = p.removeFromTop (28);
    fxPresetLabel.setBounds (fxRow.removeFromLeft (110));
    fxRow.removeFromLeft (8);
    fxPresetBox.setBounds (fxRow);
    p.removeFromTop (6);
    placeButton.setBounds (p.removeFromTop (30).removeFromLeft (140));
    p.removeFromTop (6);
    fxChoiceReadout.setBounds (p.removeFromTop (28));
    p.removeFromTop (2);
    placeHint.setBounds (p);

    auto layoutSend = [] (juce::Rectangle<int> r, juce::Label& name, juce::Slider& level, juce::Label& notes)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        name.setBounds (r.removeFromTop (26));
        r.removeFromTop (6);
        level.setBounds (r.removeFromTop (28));
        r.removeFromTop (6);
        notes.setBounds (r.removeFromTop (70));
    };
    layoutSend (sendACard, sendAName, sendALevel, sendAReadout);
    layoutSend (sendBCard, sendBName, sendBLevel, sendBReadout);
}
