#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

struct LayoutPage : public juce::Component
{
    std::function<void()> lay;
    void resized() override { if (lay) lay(); }
};

struct ControlRow : public juce::Component
{
    juce::Label label;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;

    void setup (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& paramId,
                const juce::String& title,
                const juce::String& suffix)
    {
        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);

        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 68, 20);
        if (suffix.isNotEmpty())
            slider.setTextValueSuffix (suffix);
        addAndMakeVisible (slider);

        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramId, slider);
    }

    void setupPercent (juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& paramId,
                       const juce::String& title)
    {
        setup (apvts, paramId, title, {});
        slider.setTextValueSuffix (" %");
        slider.textFromValueFunction = [] (double v)
        {
            return juce::String (juce::roundToInt (v * 100.0));
        };
        slider.valueFromTextFunction = [] (const juce::String& t)
        {
            return t.getDoubleValue() / 100.0;
        };
        slider.updateText();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromLeft (128));
        slider.setBounds (r);
    }
};

struct ToggleRow : public juce::Component
{
    juce::TextButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attach;

    void setup (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& paramId,
                const juce::String& title)
    {
        button.setButtonText (title);
        button.setClickingTogglesState (true);
        addAndMakeVisible (button);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, paramId, button);
    }

    void resized() override { button.setBounds (getLocalBounds()); }
};

struct ChoiceRow : public juce::Component
{
    juce::Label label;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;

    void setup (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& paramId,
                const juce::String& title)
    {
        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
        addAndMakeVisible (box);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, paramId, box);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromLeft (128));
        box.setBounds (r);
    }
};

class GrMeter final : public juce::Component
{
public:
    void setGrDb (float db)
    {
        grDb = db;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff12151b));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0xff2a303a));
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

        const float amount = juce::jlimit (0.0f, 1.0f, grDb / 12.0f);
        auto fill = r.reduced (4.0f);
        fill = fill.removeFromLeft (fill.getWidth() * amount);
        g.setColour (amount > 0.7f ? juce::Colour (0xffe85d5d) : juce::Colour (0xffc9a44c));
        g.fillRoundedRectangle (fill, 4.0f);

        g.setColour (juce::Colour (0xffeeeae2));
        g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
        g.drawText ("GR  " + juce::String (grDb, 1) + " dB",
                    getLocalBounds().reduced (8, 0),
                    juce::Justification::centredLeft);
    }

private:
    float grDb = 0.0f;
};
