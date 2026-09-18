#pragma once

#include "Knowledge/MixKnowledge.h"
#include <juce_gui_basics/juce_gui_basics.h>

class RecipeView final : public juce::Component
{
public:
    RecipeView();

    void setCards (std::vector<k3ch::RecipeCard> newCards);
    void paint (juce::Graphics&) override;
    void resized() override;

    int getPreferredHeight() const;

private:
    struct CardLayout
    {
        k3ch::RecipeCard card;
        juce::Rectangle<int> bounds;
        int notesY = 0;
        int notesH = 0;
    };

    void rebuildLayout();
    int measureNotes (const juce::String& notes, int width) const;

    std::vector<CardLayout> layouts;
    int preferredHeight = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecipeView)
};
