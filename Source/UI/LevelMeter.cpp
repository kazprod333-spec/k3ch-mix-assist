#include "UI/LevelMeter.h"
#include "UI/K3chLookAndFeel.h"

namespace
{
constexpr float kMinDb = -60.0f;
constexpr float kMaxDb = 0.0f;
}

LevelMeter::LevelMeter (MixAssistProcessor& p)
    : processor (p)
{
    startTimerHz (30);
}

LevelMeter::~LevelMeter()
{
    stopTimer();
}

float LevelMeter::toDb (float linear) noexcept
{
    if (linear <= 1.0e-6f)
        return kMinDb;
    return juce::jlimit (kMinDb, 6.0f, juce::Decibels::gainToDecibels (linear));
}

void LevelMeter::timerCallback()
{
    float pL = 0, pR = 0, rL = 0, rR = 0;
    processor.getMeterLevels (pL, pR, rL, rR);

    peakDbL = toDb (pL);
    peakDbR = toDb (pR);
    rmsDbL  = toDb (rL);
    rmsDbR  = toDb (rR);

    peakHoldL = juce::jmax (peakHoldL - 1.4f, peakDbL);
    peakHoldR = juce::jmax (peakHoldR - 1.4f, peakDbR);
    repaint();
}

void LevelMeter::drawBar (juce::Graphics& g, juce::Rectangle<float> bounds, float db, float holdDb) const
{
    k3ch::Palette pal;
    g.setColour (juce::Colour (0xff0a0c0f));
    g.fillRoundedRectangle (bounds, 3.0f);

    const auto t = juce::jlimit (0.0f, 1.0f, (db - kMinDb) / (kMaxDb - kMinDb));
    auto fill = bounds.withWidth (juce::jmax (1.0f, bounds.getWidth() * t));

    juce::ColourGradient grad (pal.meterOk, bounds.getX(), 0.0f,
                               pal.meterHot, bounds.getRight(), 0.0f, false);
    grad.addColour (0.72, pal.meterOk);
    grad.addColour (0.88, pal.meterWarn);
    g.setGradientFill (grad);
    g.saveState();
    g.reduceClipRegion (fill.toNearestInt());
    g.fillRoundedRectangle (bounds, 3.0f);
    g.restoreState();

    const auto ht = juce::jlimit (0.0f, 1.0f, (holdDb - kMinDb) / (kMaxDb - kMinDb));
    const float hx = bounds.getX() + bounds.getWidth() * ht;
    g.setColour (pal.text.withAlpha (0.9f));
    g.fillRect (hx - 1.0f, bounds.getY(), 2.0f, bounds.getHeight());
}

void LevelMeter::paint (juce::Graphics& g)
{
    k3ch::Palette pal;
    auto r = getLocalBounds().toFloat();
    auto readout = r.removeFromRight (62.0f);

    g.setColour (pal.muted);
    g.setFont (juce::Font (juce::FontOptions (10.5f)));

    auto label = r.removeFromLeft (28.0f);
    g.drawText ("Pk", label.removeFromTop (r.getHeight() * 0.5f), juce::Justification::centredLeft);
    g.drawText ("RMS", label, juce::Justification::centredLeft);

    auto peakArea = r.removeFromTop (r.getHeight() * 0.5f).reduced (0.0f, 3.0f);
    auto rmsArea  = r.reduced (0.0f, 3.0f);

    auto peakL = peakArea.removeFromTop (peakArea.getHeight() * 0.5f).reduced (0, 1);
    auto peakR = peakArea.reduced (0, 1);
    drawBar (g, peakL, peakDbL, peakHoldL);
    drawBar (g, peakR, peakDbR, peakHoldR);

    auto rmsL = rmsArea.removeFromTop (rmsArea.getHeight() * 0.5f).reduced (0, 1);
    auto rmsR = rmsArea.reduced (0, 1);
    drawBar (g, rmsL, rmsDbL, rmsDbL);
    drawBar (g, rmsR, rmsDbR, rmsDbR);

    const auto shown = juce::jmax (peakHoldL, peakHoldR);
    const auto dbText = shown <= kMinDb + 0.01f ? juce::String ("-inf")
                                                : juce::String (shown, 1) + " dB";
    g.setColour (processor.isBypassed() ? pal.muted : pal.gold);
    g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
    g.drawFittedText (processor.isBypassed() ? "BYPASS" : dbText,
                      readout.toNearestInt(),
                      juce::Justification::centredRight, 1);
}
