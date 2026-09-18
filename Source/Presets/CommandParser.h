#pragma once

#include "PresetLibrary.h"

namespace k3ch_presets
{

struct Command
{
    enum class Type
    {
        Unknown,
        LoadVocal,
        LoadSend,
        SetSendLevel,
        SetDryWet,
        Bypass,
        Help
    };

    Type type = Type::Unknown;
    bool ok = false;
    juce::String message;
    juce::String presetId;
    int vocalIndex = -1;
    int fxIndex = -1;
    int sendIndex = -1; // 0 = A, 1 = B
    float value = 0.0f;
    bool bypassOn = false;
};

class CommandParser
{
public:
    explicit CommandParser (const PresetLibrary& library);
    Command parse (const juce::String& text) const;

private:
    const PresetLibrary& library;
};

} // namespace k3ch_presets
