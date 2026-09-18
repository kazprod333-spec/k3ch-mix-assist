#pragma once

#include "DspEngine.h"

#include <juce_core/juce_core.h>
#include <vector>

namespace k3ch_presets
{

struct ChainStep
{
    int n = 0;
    juce::String title;
    juce::String settings;
};

struct FlInsertRow
{
    int slot = 0;
    juce::String plugin;
    juce::String setting;
    juce::String notes;
};

struct VocalPreset
{
    juce::String id;
    juce::String name;
    juce::String sourceId;
    juce::String notes;
    std::vector<juce::String> aliases;
    VocalSettings settings;
    juce::String sendAId;
    juce::String sendBId;
    std::vector<ChainStep> appliedChain;
    std::vector<FlInsertRow> flInsertPlan;

    juce::String appliedChainText() const;
    juce::String flInsertPlanText() const;
};

struct FxPreset
{
    juce::String id;
    juce::String name;
    juce::String kind;
    juce::String sourceId;
    juce::String notes;
    std::vector<juce::String> aliases;
    float defaultSendDb = -15.0f;
    SendSettings settings;
};

class PresetLibrary
{
public:
    static PresetLibrary loadEmbedded();
    static PresetLibrary parse (const juce::String& jsonText);

    bool isValid() const noexcept { return valid; }
    juce::String getError() const { return error; }
    juce::String getVersion() const { return version; }
    juce::String getStudio() const { return studio; }

    const std::vector<VocalPreset>& vocals() const { return vocalPresets; }
    const std::vector<FxPreset>& fx() const { return fxPresets; }

    juce::StringArray vocalNames() const;
    juce::StringArray fxNames() const;

    int findVocalIndex (const juce::String& query) const;
    int findFxIndex (const juce::String& query) const;

    const VocalPreset* vocalAt (int index) const;
    const FxPreset* fxAt (int index) const;
    const VocalPreset* findVocal (const juce::String& query) const;
    const FxPreset* findFx (const juce::String& query) const;

private:
    bool valid = false;
    juce::String error;
    juce::String version;
    juce::String studio { "Maison K3CH Production" };
    std::vector<VocalPreset> vocalPresets;
    std::vector<FxPreset> fxPresets;
};

} // namespace k3ch_presets
