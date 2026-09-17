#include "UI/K3chLookAndFeel.h"

namespace k3ch
{

K3chLookAndFeel::K3chLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, palette.bg);
    setColour (juce::TextButton::buttonColourId, palette.bgCard);
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2a2416));
    setColour (juce::TextButton::textColourOffId, palette.text);
    setColour (juce::TextButton::textColourOnId, palette.gold);
    setColour (juce::Label::textColourId, palette.text);
    setColour (juce::TextEditor::backgroundColourId, palette.bgCard);
    setColour (juce::TextEditor::textColourId, palette.text);
    setColour (juce::TextEditor::highlightColourId, palette.gold.withAlpha (0.35f));
    setColour (juce::TextEditor::highlightedTextColourId, palette.text);
    setColour (juce::TextEditor::outlineColourId, palette.border);
    setColour (juce::TextEditor::focusedOutlineColourId, palette.gold);
    setColour (juce::CaretComponent::caretColourId, palette.gold);
    setColour (juce::ScrollBar::thumbColourId, palette.goldDim);
    setColour (juce::ScrollBar::trackColourId, palette.bgPanel);
    setColour (juce::AlertWindow::backgroundColourId, palette.bgPanel);
    setColour (juce::AlertWindow::textColourId, palette.text);
    setColour (juce::AlertWindow::outlineColourId, palette.gold);
}

void K3chLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    auto colour = backgroundColour;

    if (shouldDrawButtonAsDown)
        colour = colour.brighter (0.12f);
    else if (shouldDrawButtonAsHighlighted)
        colour = colour.brighter (0.08f);

    g.setColour (colour);
    g.fillRoundedRectangle (bounds, 6.0f);

    if (button.getToggleState())
    {
        g.setColour (palette.gold);
        g.fillRect (bounds.removeFromLeft (3.0f).translated (1.0f, 0));
        g.setColour (palette.gold.withAlpha (0.25f));
        g.drawRoundedRectangle (button.getLocalBounds().toFloat().reduced (1.0f), 6.0f, 1.0f);
    }
    else
    {
        g.setColour (palette.border.withAlpha (0.8f));
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    }
}

void K3chLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId)
                     .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    auto r = button.getLocalBounds().reduced (12, 2);
    if (button.getToggleState())
        r.removeFromLeft (6);
    g.drawFittedText (button.getButtonText(), r, juce::Justification::centredLeft, 2);
}

juce::Font K3chLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (14.5f));
}

void K3chLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor&)
{
    g.setColour (palette.bgCard);
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 6.0f);
}

void K3chLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    g.setColour (editor.hasKeyboardFocus (true) ? palette.gold : palette.border);
    g.drawRoundedRectangle (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f, 6.0f, 1.0f);
}

} // namespace k3ch
