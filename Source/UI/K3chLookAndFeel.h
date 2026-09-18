#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace k3ch
{

struct Palette
{
    juce::Colour bg        { 0xff0b0d10 };
    juce::Colour bgPanel   { 0xff12151b };
    juce::Colour bgCard    { 0xff1a1e27 };
    juce::Colour bgSidebar { 0xff101318 };
    juce::Colour border    { 0xff2a303a };
    juce::Colour gold      { 0xffc9a44c };
    juce::Colour goldDim   { 0xff7a6430 };
    juce::Colour text      { 0xffeeeae2 };
    juce::Colour muted     { 0xff8b8680 };
    juce::Colour teal      { 0xff3dbebf };
    juce::Colour meterOk   { 0xff5cdb95 };
    juce::Colour meterWarn { 0xffe6c35c };
    juce::Colour meterHot  { 0xffe85d5d };
};

class K3chLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    K3chLookAndFeel();

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label& labelToPosition) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                        bool isMouseOver, bool isMouseDown) override;
    int getTabButtonBestWidth (juce::TabBarButton&, int tabDepth) override;

    Palette palette;
};

} // namespace k3ch
