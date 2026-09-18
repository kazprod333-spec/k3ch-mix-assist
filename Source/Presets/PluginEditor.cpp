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

PresetsEditor::PresetsEditor (PresetsProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      meter (p)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (1120, 780, 1800, 1400);
    setSize (1320, 900);

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
    colourLabel (constraintNote, lnf.palette.gold);
    constraintNote.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (constraintNote);

    colourLabel (statusLabel, lnf.palette.muted);
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
    colourLabel (chainLabel, lnf.palette.gold);
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

    sendTargetLabel.setText (juce::String::fromUTF8 ("Placer sur"), juce::dontSendNotification);
    colourLabel (sendTargetLabel, lnf.palette.gold);
    addAndMakeVisible (sendTargetLabel);
    sendTargetBox.addItem ("Send A", 1);
    sendTargetBox.addItem ("Send B", 2);
    sendTargetBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (sendTargetBox);

    fxPresetLabel.setText ("Preset FX", juce::dontSendNotification);
    colourLabel (fxPresetLabel, lnf.palette.gold);
    addAndMakeVisible (fxPresetLabel);
    fxPresetBox.setTextWhenNothingSelected ("Preset FX");
    int fxId = 1;
    for (const auto& name : processor.library.fxNames())
        fxPresetBox.addItem (name, fxId++);
    const int air = processor.library.findFxIndex ("air");
    fxPresetBox.setSelectedItemIndex (air >= 0 ? air : 0, juce::dontSendNotification);
    addAndMakeVisible (fxPresetBox);

    placeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    placeButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    placeButton.onClick = [this] { placeFxOnSelectedSend(); };
    addAndMakeVisible (placeButton);

    tabs.setOutline (0);
    tabs.setTabBarDepth (34);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, lnf.palette.bg);
    tabs.setColour (juce::TabbedComponent::outlineColourId, lnf.palette.border);
    addAndMakeVisible (tabs);

    auto addTab = [this] (const juce::String& name, LayoutPage& page)
    {
        tabs.addTab (name, lnf.palette.bgCard, &page, false);
    };
    addTab (juce::String::fromUTF8 ("Cha\xc3\xae" "ne vocale"), pageChain);
    addTab ("EQ", pageEq);
    addTab ("Dynamique", pageDyn);
    addTab ("Sat", pageSat);
    addTab ("Send A", pageSendA);
    addTab ("Send B", pageSendB);
    addTab ("Plan FL", pagePlan);

    pageChain.lay = [this] { layoutChainPage(); };
    pageEq.lay = [this] { layoutEqPage(); };
    pageDyn.lay = [this] { layoutDynPage(); };
    pageSat.lay = [this] { layoutSatPage(); };
    pageSendA.lay = [this]
    {
        auto r = pageSendA.getLocalBounds().reduced (16, 12);
        sendATitle.setBounds (r.removeFromTop (22));
        r.removeFromTop (6);
        sendABox.setBounds (r.removeFromTop (30));
        r.removeFromTop (6);
        sendALevel.setBounds (r.removeFromTop (26));
        r.removeFromTop (6);
        sendANotes.setBounds (r.removeFromTop (36));
        r.removeFromTop (8);
        ControlRow* list[] = {
            &saHpf, &saLpf, &saSmash, &saDsHz, &saDs, &saSatDrv, &saSatMix,
            &saRevMix, &saRevDec, &saRevDamp, &saDlyMix, &saDlyMs, &saDlyFb,
            &saHarshHz, &saHarshCut
        };
        const int cols = 2;
        const int rowH = 26;
        const int gap = 5;
        const int colW = (r.getWidth() - 16) / cols;
        for (int n = 0; n < 15; ++n)
        {
            const int col = n % cols;
            const int row = n / cols;
            list[n]->setBounds (r.getX() + col * (colW + 16),
                                r.getY() + row * (rowH + gap),
                                colW, rowH);
        }
    };
    pageSendB.lay = [this]
    {
        auto r = pageSendB.getLocalBounds().reduced (16, 12);
        sendBTitle.setBounds (r.removeFromTop (22));
        r.removeFromTop (6);
        sendBBox.setBounds (r.removeFromTop (30));
        r.removeFromTop (6);
        sendBLevel.setBounds (r.removeFromTop (26));
        r.removeFromTop (6);
        sendBNotes.setBounds (r.removeFromTop (36));
        r.removeFromTop (8);
        ControlRow* list[] = {
            &sbHpf, &sbLpf, &sbSmash, &sbDsHz, &sbDs, &sbSatDrv, &sbSatMix,
            &sbRevMix, &sbRevDec, &sbRevDamp, &sbDlyMix, &sbDlyMs, &sbDlyFb,
            &sbHarshHz, &sbHarshCut
        };
        const int cols = 2;
        const int rowH = 26;
        const int gap = 5;
        const int colW = (r.getWidth() - 16) / cols;
        for (int n = 0; n < 15; ++n)
        {
            const int col = n % cols;
            const int row = n / cols;
            list[n]->setBounds (r.getX() + col * (colW + 16),
                                r.getY() + row * (rowH + gap),
                                colW, rowH);
        }
    };
    pagePlan.lay = [this] { layoutPlanPage(); };

    chainHint.setText (juce::String::fromUTF8 (
                           "Pose CE plug-in sur l\xe2\x80\x99insert FL choisi. "
                           "Appliquer remplit tous les r\xc3\xa9" "glages \xe2\x80\x94 tu peux tout retoucher ensuite."),
                       juce::dontSendNotification);
    colourLabel (chainHint, lnf.palette.muted);
    chainHint.setJustificationType (juce::Justification::topLeft);
    addTo (pageChain, chainHint);
    styleReadOnly (chainView);
    addTo (pageChain, chainView);

    hpfOn.setup (processor.apvts, "hpf_on", "HPF actif");
    hpfSlope.setup (processor.apvts, "hpf_slope", "Pente");
    hpfHz.setup (processor.apvts, "hpf_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    inputDb.setup (processor.apvts, "input_db", juce::String::fromUTF8 ("Entr\xc3\xa9" "e"), " dB");
    dryWet.setupPercent (processor.apvts, "dry_wet", "Dry/Wet");
    returnMix.setupPercent (processor.apvts, "return_mix", "Return mix");
    outputDb.setup (processor.apvts, "output_db", "Sortie", " dB");
    addTo (pageChain, hpfOn);
    addTo (pageChain, hpfSlope);
    addTo (pageChain, hpfHz);
    addTo (pageChain, inputDb);
    addTo (pageChain, dryWet);
    addTo (pageChain, returnMix);
    addTo (pageChain, outputDb);

    sectionTitle (eqMudTitle, juce::String::fromUTF8 ("Mud  \xc2\xb7  cloche"), lnf.palette.gold);
    sectionTitle (eqMidTitle, juce::String::fromUTF8 ("Pr\xc3\xa9sence  \xc2\xb7  cloche"), lnf.palette.gold);
    sectionTitle (eqAirTitle, "Air  ·  shelf", lnf.palette.gold);
    eqLowHz.setup (processor.apvts, "eq_low_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    eqLowG.setup (processor.apvts, "eq_low_gain", "Gain", " dB");
    eqLowQ.setup (processor.apvts, "eq_low_q", "Facteur Q", {});
    eqMidHz.setup (processor.apvts, "eq_mid_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    eqMidG.setup (processor.apvts, "eq_mid_gain", "Gain", " dB");
    eqMidQ.setup (processor.apvts, "eq_mid_q", "Facteur Q", {});
    eqAirHz.setup (processor.apvts, "eq_air_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    eqAirG.setup (processor.apvts, "eq_air_gain", "Gain", " dB");
    for (auto* c : { &eqMudTitle, &eqMidTitle, &eqAirTitle })
        addTo (pageEq, *c);
    for (auto* c : { &eqLowHz, &eqLowG, &eqLowQ, &eqMidHz, &eqMidG, &eqMidQ, &eqAirHz, &eqAirG })
        addTo (pageEq, *c);

    sectionTitle (compTitle, "Compresseur", lnf.palette.gold);
    sectionTitle (deessTitle, "De-esser", lnf.palette.gold);
    compTh.setup (processor.apvts, "comp_threshold", "Seuil", " dB");
    compRatio.setup (processor.apvts, "comp_ratio", "Ratio", " :1");
    compAtk.setup (processor.apvts, "comp_attack", "Attaque", " ms");
    compRel.setup (processor.apvts, "comp_release", "Release", " ms");
    compMk.setup (processor.apvts, "comp_makeup", "Makeup", " dB");
    deessHz.setup (processor.apvts, "deess_hz", juce::String::fromUTF8 ("Fr\xc3\xa9quence"), " Hz");
    deessAmt.setupPercent (processor.apvts, "deess_amount", "Amount");
    addTo (pageDyn, compTitle);
    addTo (pageDyn, deessTitle);
    addTo (pageDyn, grMeter);
    for (auto* c : { &compTh, &compRatio, &compAtk, &compRel, &compMk, &deessHz, &deessAmt })
        addTo (pageDyn, *c);

    satOn.setup (processor.apvts, "sat_on", "Saturation active");
    satDrv.setupPercent (processor.apvts, "sat_drive", "Drive");
    satMix.setupPercent (processor.apvts, "sat_mix", "Mix");
    addTo (pageSat, satOn);
    addTo (pageSat, satDrv);
    addTo (pageSat, satMix);

    auto setupSend = [this] (juce::Label& title, const juce::String& text, juce::ComboBox& box,
                             const char* choiceId,
                             std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& att,
                             ControlRow& level, const char* levelId, juce::Label& notes,
                             LayoutPage& page,
                             ControlRow& hpf, ControlRow& lpf, ControlRow& smash,
                             ControlRow& dsHz, ControlRow& ds, ControlRow& sDrv, ControlRow& sMix,
                             ControlRow& rMix, ControlRow& rDec, ControlRow& rDamp,
                             ControlRow& dMix, ControlRow& dMs, ControlRow& dFb,
                             ControlRow& hHz, ControlRow& hCut,
                             const juce::String& prefix)
    {
        sectionTitle (title, text, lnf.palette.gold);
        addTo (page, title);
        addTo (page, box);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.apvts, choiceId, box);
        level.setup (processor.apvts, levelId, "Niveau", " dB");
        addTo (page, level);
        colourLabel (notes, lnf.palette.muted);
        notes.setJustificationType (juce::Justification::topLeft);
        addTo (page, notes);
        hpf.setup (processor.apvts, prefix + "hpf", "HPF", " Hz");
        lpf.setup (processor.apvts, prefix + "lpf", "LPF", " Hz");
        smash.setupPercent (processor.apvts, prefix + "smash", "Smash");
        dsHz.setup (processor.apvts, prefix + "deess_hz", "De-ess Hz", " Hz");
        ds.setupPercent (processor.apvts, prefix + "deess", "De-ess");
        sDrv.setupPercent (processor.apvts, prefix + "sat_drv", "Sat drive");
        sMix.setupPercent (processor.apvts, prefix + "sat_mix", "Sat mix");
        rMix.setupPercent (processor.apvts, prefix + "rev_mix", "Reverb mix");
        rDec.setup (processor.apvts, prefix + "rev_decay", "Decay", " s");
        rDamp.setupPercent (processor.apvts, prefix + "rev_damp", "Damp");
        dMix.setupPercent (processor.apvts, prefix + "dly_mix", "Delay mix");
        dMs.setup (processor.apvts, prefix + "dly_ms", "Delay", " ms");
        dFb.setupPercent (processor.apvts, prefix + "dly_fb", "Feedback");
        hHz.setup (processor.apvts, prefix + "harsh_hz", "Harsh Hz", " Hz");
        hCut.setup (processor.apvts, prefix + "harsh_cut", "Harsh cut", " dB");
        for (auto* c : { &hpf, &lpf, &smash, &dsHz, &ds, &sDrv, &sMix, &rMix, &rDec, &rDamp, &dMix, &dMs, &dFb, &hHz, &hCut })
            addTo (page, *c);
    };

    setupSend (sendATitle, juce::String::fromUTF8 ("Send A  \xc2\xb7  FX interne"), sendABox, "send_a_preset",
               sendABoxAttach, sendALevel, "send_a_db", sendANotes, pageSendA,
               saHpf, saLpf, saSmash, saDsHz, saDs, saSatDrv, saSatMix,
               saRevMix, saRevDec, saRevDamp, saDlyMix, saDlyMs, saDlyFb, saHarshHz, saHarshCut, "send_a_");
    setupSend (sendBTitle, juce::String::fromUTF8 ("Send B  \xc2\xb7  FX interne"), sendBBox, "send_b_preset",
               sendBBoxAttach, sendBLevel, "send_b_db", sendBNotes, pageSendB,
               sbHpf, sbLpf, sbSmash, sbDsHz, sbDs, sbSatDrv, sbSatMix,
               sbRevMix, sbRevDec, sbRevDamp, sbDlyMix, sbDlyMs, sbDlyFb, sbHarshHz, sbHarshCut, "send_b_");

    planHint.setText (juce::String::fromUTF8 (
                          "Checklist manuelle si tu veux recr\xc3\xa9" "er CLA-76, RVox, etc. sur le mixer FL."),
                      juce::dontSendNotification);
    colourLabel (planHint, lnf.palette.muted);
    addTo (pagePlan, planHint);
    copyPlanButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    copyPlanButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    copyPlanButton.onClick = [this] { copyFlInsertPlan(); };
    addTo (pagePlan, copyPlanButton);
    styleReadOnly (planView);
    addTo (pagePlan, planView);

    syncCombosFromProcessor();
    updateReadouts();
    statusLabel.setText (processor.lastStatus.isNotEmpty()
                             ? processor.lastStatus
                             : juce::String::fromUTF8 (
                                   "Cha\xc3\xae" "ne insert + r\xc3\xa9" "glages manuels \xe2\x80\x94 pas le mixer FL"),
                         juce::dontSendNotification);
    startTimerHz (8);
}

PresetsEditor::~PresetsEditor()
{
    stopTimer();
    bypassAttachment.reset();
    sendABoxAttach.reset();
    sendBBoxAttach.reset();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void PresetsEditor::addTo (juce::Component& page, juce::Component& child)
{
    page.addAndMakeVisible (child);
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

void PresetsEditor::layoutChainPage()
{
    auto r = pageChain.getLocalBounds().reduced (16, 12);
    chainHint.setBounds (r.removeFromTop (36));
    r.removeFromTop (6);
    auto left = r.removeFromLeft (juce::jmax (360, r.getWidth() * 48 / 100));
    r.removeFromLeft (16);
    chainView.setBounds (left);
    stackRows (r, hpfOn, 30);
    stackRows (r, hpfSlope, 28);
    stackRows (r, hpfHz);
    stackRows (r, inputDb);
    stackRows (r, dryWet);
    stackRows (r, returnMix);
    stackRows (r, outputDb);
}

void PresetsEditor::layoutEqPage()
{
    auto r = pageEq.getLocalBounds().reduced (16, 14);
    const int colW = (r.getWidth() - 24) / 3;
    auto mud = r.removeFromLeft (colW);
    r.removeFromLeft (12);
    auto mid = r.removeFromLeft (colW);
    r.removeFromLeft (12);
    auto air = r;
    auto layBand = [] (juce::Rectangle<int> b, juce::Label& t, ControlRow& hz, ControlRow& g, ControlRow* q)
    {
        t.setBounds (b.removeFromTop (24));
        b.removeFromTop (10);
        hz.setBounds (b.removeFromTop (28));
        b.removeFromTop (8);
        g.setBounds (b.removeFromTop (28));
        if (q != nullptr)
        {
            b.removeFromTop (8);
            q->setBounds (b.removeFromTop (28));
        }
    };
    layBand (mud, eqMudTitle, eqLowHz, eqLowG, &eqLowQ);
    layBand (mid, eqMidTitle, eqMidHz, eqMidG, &eqMidQ);
    layBand (air, eqAirTitle, eqAirHz, eqAirG, nullptr);
}

void PresetsEditor::layoutDynPage()
{
    auto r = pageDyn.getLocalBounds().reduced (16, 14);
    auto left = r.removeFromLeft ((r.getWidth() - 16) / 2);
    r.removeFromLeft (16);
    compTitle.setBounds (left.removeFromTop (22));
    left.removeFromTop (8);
    grMeter.setBounds (left.removeFromTop (32));
    left.removeFromTop (10);
    stackRows (left, compTh);
    stackRows (left, compRatio);
    stackRows (left, compAtk);
    stackRows (left, compRel);
    stackRows (left, compMk);
    deessTitle.setBounds (r.removeFromTop (22));
    r.removeFromTop (10);
    stackRows (r, deessHz);
    stackRows (r, deessAmt);
}

void PresetsEditor::layoutSatPage()
{
    auto r = pageSat.getLocalBounds().reduced (20, 18);
    r = r.removeFromLeft (juce::jmin (640, r.getWidth()));
    stackRows (r, satOn, 32);
    stackRows (r, satDrv, 28);
    stackRows (r, satMix, 28);
}

void PresetsEditor::layoutPlanPage()
{
    auto r = pagePlan.getLocalBounds().reduced (16, 12);
    auto top = r.removeFromTop (28);
    copyPlanButton.setBounds (top.removeFromRight (88));
    top.removeFromRight (8);
    planHint.setBounds (top);
    r.removeFromTop (8);
    planView.setBounds (r);
}

void PresetsEditor::timerCallback()
{
    syncCombosFromProcessor();
    updateReadouts();
    grMeter.setGrDb (processor.getCompGrDb());
    if (statusTicks > 0 && --statusTicks == 0)
        statusLabel.setText (juce::String::fromUTF8 (
                                 "Cha\xc3\xae" "ne insert + r\xc3\xa9" "glages manuels \xe2\x80\x94 pas le mixer FL"),
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
    auto fxNotes = [this] (const char* paramId) -> juce::String
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId)))
            idx = p->getIndex();
        if (const auto* fx = processor.library.fxAt (idx))
            return fx->notes;
        return {};
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

    sendANotes.setText (fxNotes ("send_a_preset"), juce::dontSendNotification);
    sendBNotes.setText (fxNotes ("send_b_preset"), juce::dontSendNotification);
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
        "Appliquer la cha\xc3\xae" "ne remplit HPF / EQ / dynamique / sat / sends.\n"
        "Tous les r\xc3\xa9" "glages restent \xc3\xa9" "ditables dans les onglets.\n"
        "Un VST3 ne pilote pas le mixer FL \xe2\x80\x94 voir Plan FL (manuel).\n\n");
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

    constraintNote.setBounds (bounds.removeFromTop (24).reduced (20, 2));

    auto strip = bounds.removeFromTop (44).reduced (16, 6);
    chainLabel.setBounds (strip.removeFromLeft (130));
    vocalBox.setBounds (strip.removeFromLeft (170));
    strip.removeFromLeft (8);
    applyChainButton.setBounds (strip.removeFromLeft (168));
    strip.removeFromLeft (16);
    sendTargetLabel.setBounds (strip.removeFromLeft (78));
    sendTargetBox.setBounds (strip.removeFromLeft (100));
    strip.removeFromLeft (8);
    fxPresetLabel.setBounds (strip.removeFromLeft (72));
    fxPresetBox.setBounds (strip.removeFromLeft (juce::jmin (180, strip.getWidth() - 100)));
    strip.removeFromLeft (8);
    placeButton.setBounds (strip.removeFromLeft (90));

    tabs.setBounds (bounds.reduced (12, 8));
}
