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
    setResizeLimits (960, 620, 1600, 1200);
    setSize (1120, 720);

    commandBox.setTextToShowWhenEmpty (
        juce::String::fromUTF8 ("optionnel : charge ModernRap  ·  mets Air sur send A"),
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

    vocalPresetLabel.setText (juce::String::fromUTF8 ("Preset insert (cha\xc3\xaene voix)"),
                              juce::dontSendNotification);
    vocalPresetLabel.setColour (juce::Label::textColourId, lnf.palette.muted);
    addAndMakeVisible (vocalPresetLabel);

    vocalBox.setTextWhenNothingSelected ("Vocal");
    int i = 1;
    for (const auto& name : processor.library.vocalNames())
        vocalBox.addItem (name, i++);
    vocalBox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        processor.loadVocalIndex (vocalBox.getSelectedItemIndex(), true);
        updateReadouts();
        statusLabel.setText (processor.lastStatus, juce::dontSendNotification);
        statusTicks = 20;
    };
    addAndMakeVisible (vocalBox);

    vocalReadout.setColour (juce::Label::textColourId, lnf.palette.muted);
    vocalReadout.setJustificationType (juce::Justification::topLeft);
    vocalReadout.setMinimumHorizontalScale (0.8f);
    addAndMakeVisible (vocalReadout);

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
    // Default to Air parallèle (skip Off at index 0) when the library has it.
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
                             : juce::String::fromUTF8 ("Insert DSP + sends internes \xe2\x80\x94 pas le mixer FL Studio"),
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

void PresetsEditor::timerCallback()
{
    syncCombosFromProcessor();
    updateReadouts();
    if (statusTicks > 0 && --statusTicks == 0)
        statusLabel.setText (juce::String::fromUTF8 ("Insert DSP + sends internes \xe2\x80\x94 pas le mixer FL Studio"),
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
    auto vocalIdx = vocalBox.getSelectedItemIndex();
    if (const auto* p = processor.library.vocalAt (vocalIdx))
        vocalReadout.setText (p->notes, juce::dontSendNotification);

    auto fxName = [this] (const char* paramId) -> juce::String
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId)))
            idx = p->getIndex();
        if (const auto* fx = processor.library.fxAt (idx))
            return fx->name;
        return "Off";
    };
    auto fxNotes = [this] (const char* paramId) -> juce::String
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId)))
            idx = p->getIndex();
        if (const auto* fx = processor.library.fxAt (idx))
            return fx->notes;
        return {};
    };

    sendAName.setText (fxName ("send_a_preset"), juce::dontSendNotification);
    sendBName.setText (fxName ("send_b_preset"), juce::dontSendNotification);
    sendAReadout.setText (fxNotes ("send_a_preset"), juce::dontSendNotification);
    sendBReadout.setText (fxNotes ("send_b_preset"), juce::dontSendNotification);
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
        "Cha\xc3\xaene voix = insert. Preset FX + Placer sur = send interne A/B.\n"
        "Les commandes et le bouton Placer pilotent CE plug-in, pas le mixer FL Studio.\n\n");
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

    card (vocalCard, juce::String::fromUTF8 ("CHA\xc3\x8eNE VOIX  \xc2\xb7  INSERT"));
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

    auto cols = bounds.reduced (16, 12);
    auto top = cols.removeFromTop (juce::jmax (250, cols.getHeight() * 54 / 100));
    cols.removeFromTop (10);
    vocalCard = top.removeFromLeft (juce::jmax (300, top.getWidth() * 38 / 100));
    top.removeFromLeft (12);
    placeCard = top;
    sendACard = cols.removeFromLeft ((cols.getWidth() - 12) / 2);
    cols.removeFromLeft (12);
    sendBCard = cols;

    auto inner = [] (juce::Rectangle<int> r)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        return r;
    };

    auto v = inner (vocalCard);
    vocalPresetLabel.setBounds (v.removeFromTop (20));
    v.removeFromTop (4);
    vocalBox.setBounds (v.removeFromTop (32));
    v.removeFromTop (8);
    vocalReadout.setBounds (v.removeFromTop (56));
    v.removeFromTop (10);
    auto mixRow = v.removeFromTop (26);
    dryLabel.setBounds (mixRow.removeFromLeft (80));
    dryWet.setBounds (mixRow);
    v.removeFromTop (6);
    mixRow = v.removeFromTop (26);
    returnLabel.setBounds (mixRow.removeFromLeft (80));
    returnMix.setBounds (mixRow);
    v.removeFromTop (6);
    mixRow = v.removeFromTop (26);
    outLabel.setBounds (mixRow.removeFromLeft (80));
    outputDb.setBounds (mixRow);

    auto p = inner (placeCard);
    auto destRow = p.removeFromTop (32);
    sendTargetLabel.setBounds (destRow.removeFromLeft (120));
    destRow.removeFromLeft (8);
    sendTargetBox.setBounds (destRow.removeFromLeft (juce::jmin (200, destRow.getWidth())));
    p.removeFromTop (10);
    auto fxRow = p.removeFromTop (32);
    fxPresetLabel.setBounds (fxRow.removeFromLeft (120));
    fxRow.removeFromLeft (8);
    fxPresetBox.setBounds (fxRow);
    p.removeFromTop (10);
    placeButton.setBounds (p.removeFromTop (34).removeFromLeft (160));
    p.removeFromTop (10);
    fxChoiceReadout.setBounds (p.removeFromTop (40));
    p.removeFromTop (4);
    placeHint.setBounds (p);

    auto layoutSend = [] (juce::Rectangle<int> r, juce::Label& name, juce::Slider& level, juce::Label& notes)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        name.setBounds (r.removeFromTop (26));
        r.removeFromTop (8);
        level.setBounds (r.removeFromTop (28));
        r.removeFromTop (8);
        notes.setBounds (r.removeFromTop (70));
    };
    layoutSend (sendACard, sendAName, sendALevel, sendAReadout);
    layoutSend (sendBCard, sendBName, sendBLevel, sendBReadout);
}
