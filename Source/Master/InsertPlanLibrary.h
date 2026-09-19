#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace k3ch_master
{

struct InsertStep
{
    int n = 0;
    juce::String plugin;
    juce::String setting;
    juce::String notes;
};

struct InsertPreset
{
    juce::String id;
    juce::String name;
    juce::String category;
    juce::String fxChainName;
    juce::String targetHint;
    juce::String notes;
    std::vector<InsertStep> steps;

    juce::String planText (int targetChannel, const juce::String& targetName) const;
};

class InsertPlanLibrary
{
public:
    static InsertPlanLibrary loadEmbedded();
    static InsertPlanLibrary parse (const juce::String& jsonText);

    bool isValid() const noexcept { return valid; }
    juce::String getError() const { return error; }
    juce::String getVersion() const { return version; }
    juce::String getStudio() const { return studio; }
    juce::String getHost() const { return host; }

    const std::vector<InsertPreset>& presets() const { return items; }
    juce::StringArray names() const;
    const InsertPreset* at (int index) const;

private:
    bool valid = false;
    juce::String error;
    juce::String version;
    juce::String studio { "Maison K3CH Production" };
    juce::String host { "Studio One 8" };
    std::vector<InsertPreset> items;
};

} // namespace k3ch_master
