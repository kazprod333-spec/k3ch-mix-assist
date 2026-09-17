#include "UI/RecipeView.h"
#include "UI/K3chLookAndFeel.h"

#include <cmath>

namespace
{
constexpr int kPad = 16;
constexpr int kTitleH = 26;
constexpr int kSubH = 18;
constexpr int kRowH = 24;
constexpr int kGap = 12;
}

RecipeView::RecipeView()
{
    setOpaque (false);
}

void RecipeView::setCards (std::vector<k3ch::RecipeCard> newCards)
{
    layouts.clear();
    layouts.reserve (newCards.size());

    for (auto& c : newCards)
    {
        CardLayout cl;
        cl.card = std::move (c);
        layouts.push_back (std::move (cl));
    }

    rebuildLayout();
    repaint();
}

int RecipeView::measureNotes (const juce::String& notes, int width) const
{
    if (notes.isEmpty() || width <= 0)
        return 0;

    juce::GlyphArrangement ga;
    ga.addJustifiedText (juce::Font (juce::FontOptions (13.0f)),
                         notes,
                         0.0f, 0.0f,
                         (float) width,
                         juce::Justification::topLeft);
    return juce::jmax (16, (int) std::ceil (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getHeight()) + 4);
}

void RecipeView::rebuildLayout()
{
    const int width = juce::jmax (200, getWidth());
    int y = 0;

    for (auto& cl : layouts)
    {
        const int innerW = width - kPad * 2;
        const bool hasSub = cl.card.subtitle.isNotEmpty();
        cl.notesH = measureNotes (cl.card.notes, innerW);
        cl.notesY = kPad + kTitleH + (hasSub ? kSubH : 0) + (int) cl.card.steps.size() * kRowH
                    + (cl.notesH > 0 ? 8 : 0);

        const int h = cl.notesY + cl.notesH + kPad;
        cl.bounds = { 0, y, width, h };
        y += h + kGap;
    }

    preferredHeight = layouts.empty() ? 220 : juce::jmax (y, 40);

    if (getHeight() != preferredHeight && width > 0)
        setSize (width, preferredHeight);
}

int RecipeView::getPreferredHeight() const
{
    return preferredHeight;
}

void RecipeView::resized()
{
    rebuildLayout();
}

void RecipeView::paint (juce::Graphics& g)
{
    k3ch::Palette pal;
    const auto titleFont = juce::Font (juce::FontOptions (16.5f).withStyle ("Bold"));
    const auto subFont   = juce::Font (juce::FontOptions (12.5f));
    const auto labelFont = juce::Font (juce::FontOptions (13.0f));
    const auto valueFont = juce::Font (juce::FontOptions (13.5f).withStyle ("Bold"));

    if (layouts.empty())
    {
        g.setColour (pal.muted);
        g.setFont (juce::Font (juce::FontOptions (15.0f)));
        g.drawFittedText ("Aucun resultat. Essayez un autre mot (Hz, dB, 808, LUFS...)",
                          getLocalBounds().reduced (24),
                          juce::Justification::centred, 3);
        return;
    }

    for (const auto& cl : layouts)
    {
        auto r = cl.bounds.toFloat();
        g.setColour (pal.bgCard);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (pal.gold);
        g.fillRect (r.getX(), r.getY() + 10.0f, 3.0f, 22.0f);
        g.setColour (pal.border.withAlpha (0.7f));
        g.drawRoundedRectangle (r, 8.0f, 1.0f);

        auto inner = cl.bounds.reduced (kPad, kPad);
        auto titleRow = inner.removeFromTop (kTitleH);
        g.setColour (pal.text);
        g.setFont (titleFont);
        g.drawFittedText (cl.card.title, titleRow, juce::Justification::centredLeft, 1);

        if (cl.card.subtitle.isNotEmpty())
        {
            auto sub = inner.removeFromTop (kSubH);
            g.setColour (pal.muted);
            g.setFont (subFont);
            g.drawFittedText (cl.card.subtitle, sub, juce::Justification::centredLeft, 1);
        }

        for (const auto& step : cl.card.steps)
        {
            auto row = inner.removeFromTop (kRowH);
            auto hintW = step.hint.isEmpty() ? 0 : juce::jmin (120, row.getWidth() / 4);
            auto hintR = hintW > 0 ? row.removeFromRight (hintW) : juce::Rectangle<int>{};
            auto valueR = row.removeFromRight (juce::jmax (90, row.getWidth() / 3));
            auto labelR = row;

            g.setColour (pal.muted);
            g.setFont (labelFont);
            g.drawFittedText (step.label, labelR, juce::Justification::centredLeft, 1);

            g.setColour (pal.gold);
            g.setFont (valueFont);
            g.drawFittedText (step.value, valueR.reduced (6, 0), juce::Justification::centredRight, 1);

            if (hintW > 0)
            {
                g.setColour (pal.teal.withAlpha (0.9f));
                g.setFont (subFont);
                g.drawFittedText (step.hint, hintR, juce::Justification::centredRight, 1);
            }
        }

        if (cl.card.notes.isNotEmpty())
        {
            inner.removeFromTop (8);
            g.setColour (pal.muted);
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            g.drawFittedText (cl.card.notes, inner.removeFromTop (cl.notesH),
                              juce::Justification::topLeft, 8);
        }
    }
}
