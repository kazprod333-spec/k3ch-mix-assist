#include "PluginEditor.h"

#include <algorithm>

MixAssistEditor::MixAssistEditor (MixAssistProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      knowledge (k3ch::MixKnowledge::loadEmbedded()),
      meter (p)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (860, 560, 1600, 1400);
    setSize (1040, 720);

    currentSectionId = processor.selectedSectionId;

    search.setTextToShowWhenEmpty ("Rechercher Hz, dB, GR, 808...", lnf.palette.muted);
    search.setJustification (juce::Justification::centredLeft);
    search.setIndents (10, 0);
    search.setEscapeAndReturnKeysConsumed (true);
    search.onTextChange = [this] { applyFilter(); };
    addAndMakeVisible (search);

    navViewport.setViewedComponent (&navColumn, false);
    navViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (navViewport);

    viewport.setViewedComponent (&recipeView, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
    addAndMakeVisible (meter);

    bypassButton.setClickingTogglesState (true);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "bypass", bypassButton);
    addAndMakeVisible (bypassButton);

    aboutButton.setButtonText (juce::String::fromUTF8 ("\xc3\x80 propos"));
    copyButton.onClick = [this] { copyRecipe(); };
    aboutButton.onClick = [this] { showAbout(); };
    copyButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2416));
    copyButton.setColour (juce::TextButton::textColourOffId, lnf.palette.gold);
    addAndMakeVisible (copyButton);
    addAndMakeVisible (aboutButton);

    statusLabel.setColour (juce::Label::textColourId, lnf.palette.muted);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    rebuildNav();

    if (knowledge.findSection (currentSectionId) == nullptr && ! knowledge.getSections().empty())
        currentSectionId = knowledge.getSections().front().id;

    showSection (currentSectionId);
    startTimerHz (4);
}

MixAssistEditor::~MixAssistEditor()
{
    stopTimer();
    bypassAttachment.reset();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void MixAssistEditor::timerCallback()
{
    if (statusTicks > 0 && --statusTicks == 0)
    {
        statusUntilClear.clear();
        applyFilter();
    }
}

void MixAssistEditor::rebuildNav()
{
    if (! navButtons.isEmpty())
        return;

    for (const auto& section : knowledge.getSections())
    {
        auto* btn = navButtons.add (new juce::TextButton (section.title));
        btn->setRadioGroupId (1);
        btn->setClickingTogglesState (true);
        btn->setConnectedEdges (juce::Button::ConnectedOnTop | juce::Button::ConnectedOnBottom);
        const auto id = section.id;
        btn->onClick = [this, id] { showSection (id); };
        navColumn.addAndMakeVisible (btn);
    }
}

void MixAssistEditor::showSection (const juce::String& id)
{
    currentSectionId = id;
    processor.selectedSectionId = id;

    for (int i = 0; i < navButtons.size(); ++i)
    {
        const bool on = knowledge.getSections()[(size_t) i].id == id;
        navButtons[i]->setToggleState (on, juce::dontSendNotification);
    }

    applyFilter();
}

void MixAssistEditor::applyFilter()
{
    rebuildNav();

    const auto query = search.getText();
    auto visible = knowledge.visibleSections (query);

    for (int i = 0; i < navButtons.size(); ++i)
    {
        const auto id = knowledge.getSections()[(size_t) i].id;
        const bool inFilter = query.trim().isEmpty()
                              || std::any_of (visible.begin(), visible.end(),
                                              [&id] (const auto& s) { return s.id == id; });
        navButtons[i]->setEnabled (inFilter);
        navButtons[i]->setAlpha (inFilter ? 1.0f : 0.35f);
    }


    if (! query.trim().isEmpty())
    {
        bool currentVisible = false;
        for (const auto& s : visible)
            if (s.id == currentSectionId)
                currentVisible = true;

        if (! currentVisible && ! visible.empty())
        {
            currentSectionId = visible.front().id;
            processor.selectedSectionId = currentSectionId;
        }
    }

    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setToggleState (knowledge.getSections()[(size_t) i].id == currentSectionId,
                                       juce::dontSendNotification);

    std::vector<k3ch::RecipeCard> cards;
    if (const auto* section = knowledge.findSection (currentSectionId))
        cards = section->filtered (query);

    recipeView.setCards (std::move (cards));
    viewport.setViewPosition (0, 0);
    resized();

    if (statusUntilClear.isNotEmpty())
        statusLabel.setText (statusUntilClear, juce::dontSendNotification);
    else if (! knowledge.isValid())
        statusLabel.setText (knowledge.getError(), juce::dontSendNotification);
    else
        statusLabel.setText ("Pass-through insert  |  Knowledge " + knowledge.getKnowledgeVersion()
                                 + "  |  local only",
                             juce::dontSendNotification);
}

void MixAssistEditor::copyRecipe()
{
    const auto text = knowledge.formatClipboard (currentSectionId, search.getText());
    juce::SystemClipboard::copyTextToClipboard (text);
    statusUntilClear = juce::String::fromUTF8 ("Recette copi\xc3\xa9""e dans le presse-papiers");
    statusTicks = 8;
    statusLabel.setText (statusUntilClear, juce::dontSendNotification);
}

void MixAssistEditor::showAbout()
{
    juce::AlertWindow::showMessageBoxAsync (
        juce::MessageBoxIconType::InfoIcon,
        "K3CH Mix Assist",
        knowledge.aboutText());
}

void MixAssistEditor::paint (juce::Graphics& g)
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
    g.drawText ("MIX ASSIST", juce::Rectangle<int> (20, 30, 280, 26), juce::Justification::centredLeft);

    g.setColour (pal.muted);
    g.setFont (juce::Font (juce::FontOptions (12.5f)));
    g.drawText (knowledge.getStudioName() + juce::String::fromUTF8 ("  \xc2\xb7  Alger"),
                juce::Rectangle<int> (20, 54, 360, 18), juce::Justification::centredLeft);

    auto footer = getLocalBounds().removeFromBottom (50);
    g.setColour (pal.bgPanel);
    g.fillRect (footer);
    g.setColour (pal.border);
    g.fillRect (footer.getX(), footer.getY(), footer.getWidth(), 1);

    auto sidebar = juce::Rectangle<int> (0, 80, 236, getHeight() - 80 - 50);
    g.setColour (pal.bgSidebar);
    g.fillRect (sidebar);
    g.setColour (pal.border);
    g.fillRect (sidebar.getRight(), sidebar.getY(), 1, sidebar.getHeight());
}

void MixAssistEditor::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop (80);
    auto footer = bounds.removeFromBottom (50);
    auto sidebar = bounds.removeFromLeft (236);

    auto headerRight = header.removeFromRight (360).reduced (16, 16);
    bypassButton.setBounds (headerRight.removeFromRight (90).reduced (4, 8));
    meter.setBounds (headerRight.reduced (4, 2));

    auto footInner = footer.reduced (16, 10);
    aboutButton.setBounds (footInner.removeFromRight (100));
    footInner.removeFromRight (8);
    copyButton.setBounds (footInner.removeFromRight (170));
    footInner.removeFromRight (12);
    statusLabel.setBounds (footInner);

    auto nav = sidebar.reduced (12, 12);
    search.setBounds (nav.removeFromTop (32));
    nav.removeFromTop (8);
    navViewport.setBounds (nav);

    const int n = navButtons.size();
    const int gap = 3;
    const int btnH = n > 12 ? 26 : 30;
    const int innerW = juce::jmax (80, navViewport.getMaximumVisibleWidth() - 2);
    int y = 0;
    for (auto* btn : navButtons)
    {
        btn->setBounds (0, y, innerW, btnH);
        y += btnH + gap;
    }
    navColumn.setSize (innerW, y);

    viewport.setBounds (bounds.reduced (16, 16));
    recipeView.setSize (juce::jmax (200, viewport.getMaximumVisibleWidth() - 4),
                        recipeView.getPreferredHeight());
}
