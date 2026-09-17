#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace k3ch
{

struct RecipeStep
{
    juce::String label;
    juce::String value;
    juce::String hint;

    juce::String haystack() const;
};

struct RecipeCard
{
    juce::String id;
    juce::String title;
    juce::String subtitle;
    std::vector<RecipeStep> steps;
    juce::String notes;

    juce::String toPlainText() const;
    bool matches (const juce::String& query) const;
    juce::String haystack() const;
};

struct KnowledgeSection
{
    juce::String id;
    juce::String title;
    int priority = 100;
    std::vector<RecipeCard> cards;

    std::vector<RecipeCard> filtered (const juce::String& query) const;
    bool matches (const juce::String& query) const;
};

/** Studio knowledge pack loaded from mix-knowledge.json (embedded).

    The UI is driven by `ui_sections`. Known section ids get richer cards;
    unknown ids (and new presets/fields) fall back to a generic JSON walker
    that formats unit suffixes (Hz, dB, GR, LUFS, …) automatically.
*/
class MixKnowledge
{
public:
    static MixKnowledge loadEmbedded();
    static MixKnowledge parse (const juce::String& jsonText);

    bool isValid() const noexcept { return valid; }
    juce::String getError() const { return error; }

    juce::String getStudioName() const { return studioName; }
    juce::String getStudioCity() const { return "Alger"; }
    juce::String getKnowledgeVersion() const { return knowledgeVersion; }
    juce::String getSource() const { return source; }

    const std::vector<KnowledgeSection>& getSections() const { return sections; }

    std::vector<KnowledgeSection> visibleSections (const juce::String& query) const;
    const KnowledgeSection* findSection (const juce::String& id) const;

    juce::String formatClipboard (const juce::String& sectionId,
                                  const juce::String& query) const;

    juce::String aboutText() const;

private:
    void buildFromRoot (const juce::var& root);
    KnowledgeSection buildSection (const juce::var& spec, const juce::var& root) const;
    std::vector<RecipeCard> cardsForSectionId (const juce::String& id, const juce::var& root) const;
    std::vector<RecipeCard> genericCards (const juce::String& title,
                                          const juce::String& id,
                                          const juce::var& value) const;

    bool valid = false;
    juce::String error;
    juce::String studioName { "Maison K3CH Production" };
    juce::String knowledgeVersion;
    juce::String source;
    std::vector<KnowledgeSection> sections;
};

} // namespace k3ch
