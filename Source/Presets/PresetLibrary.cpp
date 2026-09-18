#include "PresetLibrary.h"
#include "PresetsBinaryData.h"

#include <algorithm>

namespace k3ch_presets
{
namespace
{

float num (const juce::var& v, float fallback)
{
    if (v.isVoid() || v.isUndefined())
        return fallback;
    if (v.isDouble() || v.isInt() || v.isInt64())
        return (float) static_cast<double> (v);
    if (v.isString())
        return v.toString().getFloatValue();
    return fallback;
}

VocalSettings vocalFrom (const juce::var& params)
{
    VocalSettings s;
    auto p = [&] (const char* key, float fb) { return num (params.getProperty (key, {}), fb); };
    s.hpfHz = p ("hpf_hz", s.hpfHz);
    s.eqLowHz = p ("eq_low_hz", s.eqLowHz);
    s.eqLowGainDb = p ("eq_low_gain_db", s.eqLowGainDb);
    s.eqLowQ = p ("eq_low_q", s.eqLowQ);
    s.eqMidHz = p ("eq_mid_hz", s.eqMidHz);
    s.eqMidGainDb = p ("eq_mid_gain_db", s.eqMidGainDb);
    s.eqMidQ = p ("eq_mid_q", s.eqMidQ);
    s.eqAirHz = p ("eq_air_hz", s.eqAirHz);
    s.eqAirGainDb = p ("eq_air_gain_db", s.eqAirGainDb);
    s.compThresholdDb = p ("comp_threshold_db", s.compThresholdDb);
    s.compRatio = p ("comp_ratio", s.compRatio);
    s.compAttackMs = p ("comp_attack_ms", s.compAttackMs);
    s.compReleaseMs = p ("comp_release_ms", s.compReleaseMs);
    s.compMakeupDb = p ("comp_makeup_db", s.compMakeupDb);
    s.deessHz = p ("deess_hz", s.deessHz);
    s.deessAmount = p ("deess_amount", s.deessAmount);
    s.satDrive = p ("sat_drive", s.satDrive);
    s.satMix = p ("sat_mix", s.satMix);
    return s;
}

SendSettings sendFrom (const juce::String& kind, const juce::var& params)
{
    SendSettings s;
    s.active = ! kind.equalsIgnoreCase ("off");
    auto p = [&] (const char* key, float fb) { return num (params.getProperty (key, {}), fb); };
    s.hpfHz = p ("hpf_hz", s.hpfHz);
    s.lpfHz = p ("lpf_hz", s.lpfHz);
    s.smash = p ("smash", s.smash);
    s.deessHz = p ("deess_hz", s.deessHz);
    s.deessAmount = p ("deess_amount", s.deessAmount);
    s.satDrive = p ("sat_drive", s.satDrive);
    s.satMix = p ("sat_mix", s.satMix);
    s.reverbMix = p ("reverb_mix", s.reverbMix);
    s.reverbDecayS = p ("reverb_decay_s", s.reverbDecayS);
    s.reverbDamp = p ("reverb_damp", s.reverbDamp);
    s.delayMix = p ("delay_mix", s.delayMix);
    s.delayMs = p ("delay_ms", s.delayMs);
    s.delayFeedback = p ("delay_feedback", s.delayFeedback);
    s.harshHz = p ("harsh_hz", s.harshHz);
    s.harshCutDb = p ("harsh_cut_db", s.harshCutDb);
    return s;
}

std::vector<juce::String> readAliases (const juce::var& node, const juce::String& id, const juce::String& name)
{
    std::vector<juce::String> out;
    out.push_back (id);
    out.push_back (name);
    if (auto* arr = node.getProperty ("aliases", {}).getArray())
        for (const auto& a : *arr)
            out.push_back (a.toString());
    return out;
}

juce::String fold (juce::String s)
{
    s = s.toLowerCase();
    const juce::String pairs[][2] = {
        { juce::String::charToString (juce::juce_wchar (0x00E9)), "e" },
        { juce::String::charToString (juce::juce_wchar (0x00E8)), "e" },
        { juce::String::charToString (juce::juce_wchar (0x00EA)), "e" },
        { juce::String::charToString (juce::juce_wchar (0x00EB)), "e" },
        { juce::String::charToString (juce::juce_wchar (0x00E0)), "a" },
        { juce::String::charToString (juce::juce_wchar (0x00E2)), "a" },
        { juce::String::charToString (juce::juce_wchar (0x00E7)), "c" },
        { juce::String::charToString (juce::juce_wchar (0x00F4)), "o" },
        { juce::String::charToString (juce::juce_wchar (0x00F9)), "u" },
        { juce::String::charToString (juce::juce_wchar (0x00FB)), "u" },
        { juce::String::charToString (juce::juce_wchar (0x00EE)), "i" },
        { juce::String::fromUTF8 ("\xc3\xab"), "e" }
    };
    for (const auto& p : pairs)
        s = s.replace (p[0], p[1]);
    return s.trim();
}

} // namespace

PresetLibrary PresetLibrary::loadEmbedded()
{
    const auto text = juce::String::fromUTF8 (PresetsBinaryData::presetsruntime_json,
                                              PresetsBinaryData::presetsruntime_jsonSize);
    return parse (text);
}

PresetLibrary PresetLibrary::parse (const juce::String& jsonText)
{
    PresetLibrary lib;
    auto root = juce::JSON::parse (jsonText);
    if (root.isVoid())
    {
        lib.error = "Impossible de parser presets-runtime.json";
        return lib;
    }

    lib.version = root.getProperty ("version", {}).toString();
    const auto studio = root.getProperty ("studio", {}).toString();
    if (studio.isNotEmpty())
        lib.studio = studio;

    if (auto* arr = root.getProperty ("vocal_presets", {}).getArray())
    {
        for (const auto& item : *arr)
        {
            VocalPreset p;
            p.id = item.getProperty ("id", {}).toString();
            p.name = item.getProperty ("name", {}).toString();
            p.sourceId = item.getProperty ("source_id", {}).toString();
            p.notes = item.getProperty ("notes", {}).toString();
            p.aliases = readAliases (item, p.id, p.name);
            p.settings = vocalFrom (item.getProperty ("params", {}));
            if (p.id.isNotEmpty())
                lib.vocalPresets.push_back (std::move (p));
        }
    }

    if (auto* arr = root.getProperty ("fx_presets", {}).getArray())
    {
        for (const auto& item : *arr)
        {
            FxPreset p;
            p.id = item.getProperty ("id", {}).toString();
            p.name = item.getProperty ("name", {}).toString();
            p.kind = item.getProperty ("kind", {}).toString();
            p.sourceId = item.getProperty ("source_id", {}).toString();
            p.notes = item.getProperty ("notes", {}).toString();
            p.defaultSendDb = num (item.getProperty ("default_send_db", {}), -15.0f);
            p.aliases = readAliases (item, p.id, p.name);
            p.settings = sendFrom (p.kind, item.getProperty ("params", {}));
            if (p.id.isNotEmpty())
                lib.fxPresets.push_back (std::move (p));
        }
    }

    lib.valid = ! lib.vocalPresets.empty() && ! lib.fxPresets.empty();
    if (! lib.valid)
        lib.error = "Bibliotheque presets vide";
    return lib;
}

juce::StringArray PresetLibrary::vocalNames() const
{
    juce::StringArray names;
    for (const auto& p : vocalPresets)
        names.add (p.name);
    return names;
}

juce::StringArray PresetLibrary::fxNames() const
{
    juce::StringArray names;
    for (const auto& p : fxPresets)
        names.add (p.name);
    return names;
}

int PresetLibrary::findVocalIndex (const juce::String& query) const
{
    const auto q = fold (query);
    if (q.isEmpty())
        return -1;

    int best = -1;
    int bestLen = -1;
    for (int i = 0; i < (int) vocalPresets.size(); ++i)
    {
        for (const auto& a : vocalPresets[(size_t) i].aliases)
        {
            const auto f = fold (a);
            if (f == q)
                return i;
            if ((q.contains (f) || f.contains (q)) && (int) f.length() > bestLen)
            {
                best = i;
                bestLen = (int) f.length();
            }
        }
    }
    return best;
}

int PresetLibrary::findFxIndex (const juce::String& query) const
{
    const auto q = fold (query);
    if (q.isEmpty())
        return -1;

    int exact = -1;
    int best = -1;
    int bestLen = -1;
    for (int i = 0; i < (int) fxPresets.size(); ++i)
    {
        for (const auto& a : fxPresets[(size_t) i].aliases)
        {
            const auto f = fold (a);
            if (f == q)
            {
                // Prefer longer exact aliases ("air parallele" over "air" if equal - exact wins)
                if (exact < 0 || (int) f.length() > (int) fold (fxPresets[(size_t) exact].aliases.front()).length())
                    exact = i;
                exact = i;
            }
            else if ((q.contains (f) || f.contains (q)) && (int) f.length() > bestLen)
            {
                best = i;
                bestLen = (int) f.length();
            }
        }
    }
    return exact >= 0 ? exact : best;
}

const VocalPreset* PresetLibrary::vocalAt (int index) const
{
    if (index < 0 || index >= (int) vocalPresets.size())
        return nullptr;
    return &vocalPresets[(size_t) index];
}

const FxPreset* PresetLibrary::fxAt (int index) const
{
    if (index < 0 || index >= (int) fxPresets.size())
        return nullptr;
    return &fxPresets[(size_t) index];
}

const VocalPreset* PresetLibrary::findVocal (const juce::String& query) const
{
    return vocalAt (findVocalIndex (query));
}

const FxPreset* PresetLibrary::findFx (const juce::String& query) const
{
    return fxAt (findFxIndex (query));
}

} // namespace k3ch_presets
