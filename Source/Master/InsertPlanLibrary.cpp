#include "InsertPlanLibrary.h"
#include "MasterBinaryData.h"

namespace k3ch_master
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
} // namespace

juce::String InsertPreset::planText (int targetChannel, const juce::String& targetName) const
{
    juce::String t;
    t += juce::String::fromUTF8 ("Plan FX Chain Studio Pro 8 \xe2\x80\x94 ") + name + "\n";
    t += juce::String::fromUTF8 ("Nom \xc3\xa0 enregistrer : ") + fxChainName + "\n";
    t += juce::String::fromUTF8 ("Canal cible (m\xc3\xa9mo) : #") + juce::String (targetChannel);
    if (targetName.trim().isNotEmpty())
        t += juce::String::fromUTF8 ("  \xc2\xb7  ") + targetName.trim();
    t += "\n";
    if (targetHint.isNotEmpty())
        t += juce::String::fromUTF8 ("Hint : ") + targetHint + "\n";
    t += "\n";
    t += juce::String::fromUTF8 (
        "Un VST3 ne peut pas charger ces plugins sur un autre canal Console.\n"
        "1) S\xc3\xa9lectionne le canal dans Studio Pro 8.\n"
        "2) Pose K3CH Presets sur CE canal pour la cha\xc3\xae" "ne live (si voix).\n"
        "3) Recr\xc3\xa9" "e les inserts ci-dessous \xc3\xa0 la main, puis Store FX Chain.\n"
        "4) Rappelle la cha\xc3\xae" "ne via le Navigateur ou une macro Macro Organizer.\n\n");

    for (const auto& step : steps)
    {
        t += juce::String (step.n) + ".  " + step.plugin;
        if (step.setting.isNotEmpty())
            t += juce::String::fromUTF8 ("  \xe2\x80\x94  ") + step.setting;
        if (step.notes.isNotEmpty())
            t += "  (" + step.notes + ")";
        t += "\n";
    }

    if (notes.isNotEmpty())
    {
        t += "\n";
        t += juce::String::fromUTF8 ("Notes : ") + notes + "\n";
    }
    return t.trimEnd();
}

InsertPlanLibrary InsertPlanLibrary::loadEmbedded()
{
    const auto text = juce::String::fromUTF8 (MasterBinaryData::masterinserts_json,
                                              MasterBinaryData::masterinserts_jsonSize);
    return parse (text);
}

InsertPlanLibrary InsertPlanLibrary::parse (const juce::String& jsonText)
{
    InsertPlanLibrary lib;
    auto root = juce::JSON::parse (jsonText);
    if (root.isVoid())
    {
        lib.error = "Impossible de parser master-inserts.json";
        return lib;
    }

    lib.version = root.getProperty ("version", {}).toString();
    const auto studio = root.getProperty ("studio", {}).toString();
    if (studio.isNotEmpty())
        lib.studio = studio;
    const auto host = root.getProperty ("host", {}).toString();
    if (host.isNotEmpty())
        lib.host = host;

    if (auto* arr = root.getProperty ("insert_presets", {}).getArray())
    {
        for (const auto& item : *arr)
        {
            InsertPreset p;
            p.id = item.getProperty ("id", {}).toString();
            p.name = item.getProperty ("name", {}).toString();
            p.category = item.getProperty ("category", {}).toString();
            p.fxChainName = item.getProperty ("fx_chain_name", {}).toString();
            p.targetHint = item.getProperty ("target_hint", {}).toString();
            p.notes = item.getProperty ("notes", {}).toString();
            if (auto* steps = item.getProperty ("steps", {}).getArray())
            {
                for (const auto& st : *steps)
                {
                    InsertStep s;
                    s.n = juce::roundToInt (num (st.getProperty ("n", {}), (float) p.steps.size() + 1.0f));
                    s.plugin = st.getProperty ("plugin", {}).toString();
                    s.setting = st.getProperty ("setting", {}).toString();
                    s.notes = st.getProperty ("notes", {}).toString();
                    if (s.plugin.isNotEmpty())
                        p.steps.push_back (std::move (s));
                }
            }
            if (p.id.isNotEmpty())
                lib.items.push_back (std::move (p));
        }
    }

    lib.valid = ! lib.items.empty();
    if (! lib.valid)
        lib.error = "Bibliotheque de plans insert vide";
    return lib;
}

juce::StringArray InsertPlanLibrary::names() const
{
    juce::StringArray out;
    for (const auto& p : items)
        out.add (p.name);
    return out;
}

const InsertPreset* InsertPlanLibrary::at (int index) const
{
    if (index < 0 || index >= (int) items.size())
        return nullptr;
    return &items[(size_t) index];
}

} // namespace k3ch_master
