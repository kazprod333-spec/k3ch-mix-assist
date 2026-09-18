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
    setColour (juce::ComboBox::backgroundColourId, palette.bgCard);
    setColour (juce::ComboBox::textColourId, palette.text);
    setColour (juce::ComboBox::outlineColourId, palette.border);
    setColour (juce::ComboBox::arrowColourId, palette.gold);
    setColour (juce::ComboBox::focusedOutlineColourId, palette.gold);
    setColour (juce::PopupMenu::backgroundColourId, palette.bgPanel);
    setColour (juce::PopupMenu::textColourId, palette.text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff2a2416));
    setColour (juce::PopupMenu::highlightedTextColourId, palette.gold);
    setColour (juce::Slider::backgroundColourId, palette.bg);
    setColour (juce::Slider::trackColourId, palette.goldDim);
    setColour (juce::Slider::thumbColourId, palette.gold);
    setColour (juce::Slider::textBoxTextColourId, palette.text);
    setColour (juce::Slider::textBoxBackgroundColourId, palette.bgCard);
    setColour (juce::Slider::textBoxOutlineColourId, palette.border);
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

void K3chLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (palette.bgCard);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (box.hasKeyboardFocus (true) ? palette.gold : palette.border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    const float ah = 5.0f, aw = 7.0f;
    juce::Path p;
    p.addTriangle (width - 16.0f - aw * 0.5f, height * 0.5f - ah * 0.35f,
                   width - 16.0f + aw * 0.5f, height * 0.5f - ah * 0.35f,
                   width - 16.0f, height * 0.5f + ah * 0.55f);
    g.setColour (palette.gold);
    g.fillPath (p);
}

void K3chLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& labelToPosition)
{
    labelToPosition.setBounds (10, 1, box.getWidth() - 30, box.getHeight() - 2);
    labelToPosition.setFont (getComboBoxFont (box));
    labelToPosition.setJustificationType (juce::Justification::centredLeft);
}

juce::Font K3chLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (14.5f));
}

void K3chLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (palette.bgPanel);
    g.setColour (palette.gold.withAlpha (0.45f));
    g.drawRect (0, 0, width, height, 1);
}

void K3chLookAndFeel::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                     bool isMouseOver, bool)
{
    auto r = button.getLocalBounds().toFloat();
    const bool front = button.isFrontTab();
    g.setColour (front ? palette.bgCard : (isMouseOver ? palette.bgCard.brighter (0.04f) : palette.bgSidebar));
    g.fillRect (r);
    if (front)
    {
        g.setColour (palette.gold);
        g.fillRect (r.removeFromBottom (2.0f));
    }
    g.setColour (front ? palette.gold : palette.muted);
    g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle (front ? "Bold" : "Regular")));
    g.drawText (button.getButtonText(), button.getLocalBounds().reduced (6, 0),
                juce::Justification::centred);
}

int K3chLookAndFeel::getTabButtonBestWidth (juce::TabBarButton& button, int)
{
    return 28 + button.getButtonText().length() * 8;
}

} // namespace k3ch
