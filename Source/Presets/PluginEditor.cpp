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
    setResizeLimits (900, 560, 1600, 1200);
    setSize (1080, 680);

    commandBox.setTextToShowWhenEmpty (
        juce::String::fromUTF8 ("charge ModernRap  ·  mets Air parall\xc3\xa8le sur send A  ·  harsh sur send B"),
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
    };
    addAndMakeVisible (vocalBox);

    vocalReadout.setColour (juce::Label::textColourId, lnf.palette.muted);
    vocalReadout.setJustificationType (juce::Justification::topLeft);
    vocalReadout.setMinimumHorizontalScale (0.8f);
    addAndMakeVisible (vocalReadout);

    auto fillFx = [this] (juce::ComboBox& box)
    {
        box.clear (juce::dontSendNotification);
        int id = 1;
        for (const auto& name : processor.library.fxNames())
            box.addItem (name, id++);
    };
    fillFx (sendABox);
    fillFx (sendBBox);

    sendABox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        processor.loadFxIndex (0, sendABox.getSelectedItemIndex(), true, true);
        updateReadouts();
    };
    sendBBox.onChange = [this]
    {
        if (ignoreCombo)
            return;
        processor.loadFxIndex (1, sendBBox.getSelectedItemIndex(), true, true);
        updateReadouts();
    };
    addAndMakeVisible (sendABox);
    addAndMakeVisible (sendBBox);

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
    dryWet.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)); };
    dryWet.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };
    setupMix (returnMix, " %");
    returnMix.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)); };
    returnMix.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };
    setupMix (outputDb, " dB");

    dryAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "dry_wet", dryWet);
    retAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "return_mix", returnMix);
    outAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "output_db", outputDb);

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
    sendABox.setSelectedItemIndex (choiceIndex ("send_a_preset"), juce::dontSendNotification);
    sendBBox.setSelectedItemIndex (choiceIndex ("send_b_preset"), juce::dontSendNotification);
    ignoreCombo = false;
}

void PresetsEditor::updateReadouts()
{
    auto vocalIdx = vocalBox.getSelectedItemIndex();
    if (const auto* p = processor.library.vocalAt (vocalIdx))
        vocalReadout.setText (p->notes, juce::dontSendNotification);

    auto fxText = [this] (int idx) -> juce::String
    {
        if (const auto* p = processor.library.fxAt (idx))
            return p->notes;
        return {};
    };
    sendAReadout.setText (fxText (sendABox.getSelectedItemIndex()), juce::dontSendNotification);
    sendBReadout.setText (fxText (sendBBox.getSelectedItemIndex()), juce::dontSendNotification);
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
        "Cha\xc3\xaene voix (HPF / EQ / comp / de-esser / sat) + 2 sends internes A/B.\n"
        "Les commandes pilotent CE plug-in, pas le mixer FL Studio (impossible en VST3).\n\n");
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
    auto cmdBar = body.removeFromTop (86);
    g.setColour (pal.bgSidebar);
    g.fillRect (cmdBar);
    g.setColour (pal.border);
    g.fillRect (cmdBar.getX(), cmdBar.getBottom() - 1, cmdBar.getWidth(), 1);

    g.setColour (pal.goldDim);
    g.setFont (juce::Font (juce::FontOptions (11.5f).withStyle ("Bold")));
    g.drawText ("COMMANDE", juce::Rectangle<int> (20, 88, 200, 16), juce::Justification::centredLeft);

    auto cols = body.reduced (16, 16);
    auto left = cols.removeFromLeft (juce::jmax (280, cols.getWidth() * 38 / 100));
    cols.removeFromLeft (12);
    auto sendA = cols.removeFromLeft (cols.getWidth() / 2);
    cols.removeFromLeft (10);

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

    card (left, juce::String::fromUTF8 ("CHA\xc3\x8eNE VOIX"));
    card (sendA, "SEND A");
    card (cols, "SEND B");
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

    auto cmdBar = bounds.removeFromTop (86).reduced (16, 10);
    cmdBar.removeFromTop (14);
    auto cmdRow = cmdBar.removeFromTop (34);
    applyButton.setBounds (cmdRow.removeFromRight (120));
    cmdRow.removeFromRight (8);
    commandBox.setBounds (cmdRow);

    auto cols = bounds.reduced (16, 16);
    auto left = cols.removeFromLeft (juce::jmax (280, cols.getWidth() * 38 / 100));
    cols.removeFromLeft (12);
    auto sendA = cols.removeFromLeft (cols.getWidth() / 2);
    cols.removeFromLeft (10);
    auto sendB = cols;

    auto layoutCard = [] (juce::Rectangle<int> r)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        return r;
    };

    auto v = layoutCard (left);
    vocalBox.setBounds (v.removeFromTop (32));
    v.removeFromTop (10);
    vocalReadout.setBounds (v.removeFromTop (70));
    v.removeFromTop (12);
    auto mixRow = v.removeFromTop (28);
    dryLabel.setBounds (mixRow.removeFromLeft (80));
    dryWet.setBounds (mixRow);
    v.removeFromTop (8);
    mixRow = v.removeFromTop (28);
    returnLabel.setBounds (mixRow.removeFromLeft (80));
    returnMix.setBounds (mixRow);
    v.removeFromTop (8);
    mixRow = v.removeFromTop (28);
    outLabel.setBounds (mixRow.removeFromLeft (80));
    outputDb.setBounds (mixRow);

    auto layoutSend = [] (juce::Rectangle<int> r, juce::ComboBox& box, juce::Slider& level, juce::Label& notes)
    {
        r = r.reduced (16, 12);
        r.removeFromTop (26);
        box.setBounds (r.removeFromTop (32));
        r.removeFromTop (10);
        level.setBounds (r.removeFromTop (28));
        r.removeFromTop (10);
        notes.setBounds (r.removeFromTop (90));
    };
    layoutSend (sendA, sendABox, sendALevel, sendAReadout);
    layoutSend (sendB, sendBBox, sendBLevel, sendBReadout);
}
