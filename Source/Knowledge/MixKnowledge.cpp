#include "Knowledge/MixKnowledge.h"
#include "BinaryData.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>

namespace k3ch
{
namespace
{

juce::String lower (const juce::String& s) { return s.toLowerCase(); }

bool isNumber (const juce::var& v)
{
    return v.isInt() || v.isInt64() || v.isDouble();
}

double asNumber (const juce::var& v) { return static_cast<double> (v); }

bool isNullish (const juce::var& v)
{
    return v.isVoid() || v.isUndefined();
}

juce::String formatNumber (double n)
{
    if (! std::isfinite (n))
        return "?";

    if (std::abs (n - std::round (n)) < 1.0e-4)
        return juce::String (juce::roundToInt (n));

    auto s = juce::String (n, 2);

    if (s.containsChar ('.'))
    {
        while (s.endsWithChar ('0'))
            s = s.dropLastCharacters (1);

        if (s.endsWithChar ('.'))
            s = s.dropLastCharacters (1);
    }

    return s;
}

juce::String formatSignedDb (double n)
{
    const auto body = formatNumber (n);
    if (n > 0.0 && ! body.startsWithChar ('+'))
        return "+" + body + " dB";
    return body + " dB";
}

bool isMetaKey (const juce::String& key)
{
    return key == "id" || key == "priority" || key == "ui_sections"
        || key == "docs_pending_ingest" || key == "source" || key == "sources"
        || key == "version" || key == "locale" || key == "gaps"
        || key == "file" || key == "used_for" || key == "parent";
}

bool skipAsStep (const juce::String& key)
{
    return isMetaKey (key)
        || key == "name" || key == "notes" || key == "label" || key == "title"
        || key == "problem" || key == "style" || key == "genre"
        || key == "track_type" || key == "daw" || key == "fiche"
        || key == "interdit" || key == "family";
}

juce::String unitForKey (const juce::String& key)
{
    const auto k = lower (key);

    if (k.endsWith ("_gr_db") || k.contains ("gr_db") || k.endsWith ("_gr"))
        return "dB GR";
    if (k.endsWith ("_dbfs") || k.contains ("dbfs"))
        return "dBFS";
    if (k.endsWith ("_dbtp") || k.contains ("dbtp"))
        return "dBTP";
    if (k.endsWith ("_lufs") || k.contains ("lufs"))
        return "LUFS";
    if (k.endsWith ("_khz") || k.contains ("_khz"))
        return "kHz";
    if (k.endsWith ("_hz") || k == "hz")
        return "Hz";
    if (k.endsWith ("_db") || k.contains ("gain_db") || k.contains ("sidechain_db"))
        return "dB";
    if (k.endsWith ("_minutes") || k.endsWith ("_min"))
        return "min";
    if (k.endsWith ("_samples"))
        return "samples";
    if (k.endsWith ("_s") && ! k.endsWith ("_samples") && ! k.endsWith ("notes"))
        return "s";
    if (k.endsWith ("_ms"))
        return "ms";
    if (k.endsWith ("_pct") || k.contains ("percent"))
        return "%";
    if (k.endsWith ("_oct") || k.contains ("slope"))
        return "dB/oct";
    if (k.contains ("amount"))
        return "%";

    return {};
}

juce::String humanizeKey (juce::String key)
{
    static const std::map<juce::String, juce::String> known {
        { "hpf_hz", "HPF" },
        { "eq_bands", "EQ" },
        { "chain_steps", utf8 ("Cha\xc3\xaene") },
        { "send_db", "Send" },
        { "plugin_stacks", "Stacks plugins" },
        { "parallel_buses", utf8 ("Bus parall\xc3\xa8les") },
        { "checklists", "Checklists" },
        { "eq_guide", "EQ voix" },
        { "eq_recipes_by_style", "Recettes EQ par style" },
        { "hpf_defaults", utf8 ("HPF d\xc3\xa9" "fauts") },
        { "eight_oh_eight", "808" },
        { "autotune_guide", "Auto-Tune" },
        { "sends_guide", "Sends" },
        { "pipeline_rules", "Pipeline" },
        { "core_plugin_stack", utf8 ("Stack c\xc5\x93ur") },
        { "tune_amount", "Tune" },
        { "comp_fet", "Comp FET" },
        { "comp_leveler", "Comp leveler" },
        { "deess_khz", "De-ess" },
        { "reverb_decay_s", "Reverb decay" },
        { "delay", "Delay" },
        { "thump_hz", "Thump" },
        { "click_khz", "Click" },
        { "cut_cardboard_hz", "Cut cardboard" },
        { "eight_oh_eight", "808" },
        { "kick", "Kick" },
        { "mono_below_hz", "Mono below" },
        { "kick_sidechain_db", "Kick sidechain" },
        { "saturation_before_bass_bus", "Saturation before bass bus" },
        { "vocal_fet_insert_gr_db", "Vocal FET insert" },
        { "vocal_opto_rvox_gr_db", "Vocal opto / RVox" },
        { "bus_ssl_vca_glue_gr_db", "Bus SSL VCA glue" },
        { "true_peak_max_dbtp", "True peak max" },
        { "target_lufs", "Target LUFS" },
        { "target_rms_dbfs", "Target RMS" },
        { "static_balance_minutes", "Static balance" },
        { "static_balance_rule", "Static balance rule" },
        { "check_mono_and_references_every_minutes", "Check mono & refs every" },
        { "sample_rate_hz", "Sample rate" },
        { "bit_depth", "Bit depth" },
        { "input_peaks_dbfs", "Input peaks" },
        { "buffer_samples", "Buffer" },
        { "direct_monitoring", "Direct monitoring" },
        { "recording_daw", "Recording DAW" },
        { "mixing_daw", "Mixing DAW" },
        { "notes", "Notes" },
        { "separate_session", "Separate session" },
        { "limiter", "Limiter" },
        { "export", "Export" },
        { "model", "Model" },
        { "filtered", "Filtered" },
        { "note", "Note" },
        { "name", "Name" },
        { "workflow", "Workflow" },
        { "recording", utf8 ("Enregistrement") },
        { "gain_staging", "Gain staging" },
        { "vocal_chain_order", utf8 ("Ordre de cha\xc3\xaene voix") },
        { "compression_guide", "Compression" },
        { "mastering", "Mastering" },
        { "presets", "Presets" },
        { "drums", "Drums" },
        { "studio", "Studio" }
    };

    if (auto it = known.find (key); it != known.end())
        return it->second;

    for (const auto* suffix : { "_gr_db", "_dbfs", "_dbtp", "_lufs", "_khz", "_hz",
                                "_db", "_minutes", "_samples", "_s", "_amount" })
    {
        if (key.endsWithIgnoreCase (suffix))
        {
            key = key.dropLastCharacters ((int) std::strlen (suffix));
            break;
        }
    }

    key = key.replaceCharacter ('_', ' ');
    juce::String out;
    auto parts = juce::StringArray::fromTokens (key, " ", {});

    static const juce::StringArray caps { "hpf", "lpf", "eq", "rms", "lufs", "wav",
                                          "fet", "gr", "ssl", "vca", "daw", "q" };

    for (auto& p : parts)
    {
        if (p.isEmpty())
            continue;

        if (caps.contains (p.toLowerCase()))
            p = p.toUpperCase();
        else
            p = p.substring (0, 1).toUpperCase() + p.substring (1).toLowerCase();

        if (out.isNotEmpty())
            out += " ";
        out += p;
    }

    return out.isNotEmpty() ? out : key;
}

juce::String withUnit (const juce::String& numberText, const juce::String& unit)
{
    if (unit.isEmpty())
        return numberText;
    return numberText + " " + unit;
}

juce::String formatRange (const juce::Array<juce::var>* arr, const juce::String& unit)
{
    if (arr == nullptr || arr->size() != 2 || ! isNumber ((*arr)[0]) || ! isNumber ((*arr)[1]))
        return {};

    return withUnit (formatNumber (asNumber ((*arr)[0]))
                         + juce::String::fromUTF8 ("\xe2\x80\x93")
                         + formatNumber (asNumber ((*arr)[1])),
                     unit);
}

juce::String formatScalar (const juce::var& v, const juce::String& key)
{
    if (v.isBool())
        return static_cast<bool> (v) ? "Oui" : "Non";

    if (v.isString())
        return v.toString();

    if (isNumber (v))
    {
        const auto n = asNumber (v);
        const auto unit = unitForKey (key);

        if (unit == "dB" && (lower (key).contains ("gain") || n > 0.0)
            && ! lower (key).contains ("max") && ! lower (key).contains ("peak"))
            return formatSignedDb (n);

        return withUnit (formatNumber (n), unit);
    }

    return v.toString();
}

juce::String formatEqBand (const juce::var& band)
{
    auto* obj = band.getDynamicObject();
    if (obj == nullptr)
        return band.toString();

    const auto type = obj->getProperty ("type").toString();
    const auto freq = obj->getProperty ("freq_hz");
    const auto gain = obj->getProperty ("gain_db");
    const auto q    = obj->getProperty ("q");
    const auto notes = obj->getProperty ("notes").toString();

    juce::String t = type.isNotEmpty()
                         ? (type.substring (0, 1).toUpperCase() + type.substring (1).replaceCharacter ('_', ' '))
                         : "EQ";

    if (auto* range = freq.getArray())
        t += " " + formatRange (range, "Hz");
    else if (isNumber (freq))
        t += " " + formatNumber (asNumber (freq)) + " Hz";

    juce::String value;
    if (isNumber (gain))
        value = formatSignedDb (asNumber (gain));
    else if (notes.isNotEmpty())
        value = notes;

    juce::String hint;
    if (isNumber (q))
        hint = "Q " + formatNumber (asNumber (q));
    if (notes.isNotEmpty() && isNumber (gain))
        hint = hint.isEmpty() ? notes : hint + " " + notes;

    if (hint.isNotEmpty() && value.isNotEmpty())
        return t + utf8 ("  \xc2\xb7  ") + value + utf8 ("  \xc2\xb7  ") + hint;
    if (value.isNotEmpty())
        return t + utf8 ("  \xc2\xb7  ") + value;
    return t;
}

std::vector<RecipeStep> stepsFromVar (const juce::String& key, const juce::var& v);

RecipeStep step (const juce::String& label, const juce::String& value, const juce::String& hint = {})
{
    return { label, value, hint };
}

std::vector<RecipeStep> stepsFromObject (juce::DynamicObject& obj)
{
    std::vector<RecipeStep> steps;

    for (const auto& p : obj.getProperties())
    {
        const auto key = p.name.toString();
        if (skipAsStep (key))
            continue;

        auto more = stepsFromVar (key, p.value);
        steps.insert (steps.end(), more.begin(), more.end());
    }

    return steps;
}

bool arrayIsNumericRange (const juce::Array<juce::var>* arr)
{
    return arr != nullptr && arr->size() == 2 && isNumber ((*arr)[0]) && isNumber ((*arr)[1]);
}

bool arrayOfObjects (const juce::Array<juce::var>* arr)
{
    if (arr == nullptr || arr->isEmpty())
        return false;
    return (*arr)[0].isObject();
}

RecipeStep stepFromRecord (const juce::var& item, int index, const juce::String& fallbackLabel)
{
    auto* o = item.getDynamicObject();
    if (o == nullptr)
        return step (fallbackLabel, item.toString());

    const auto plugin  = o->getProperty ("plugin").toString();
    const auto family  = o->getProperty ("family").toString();
    const auto title   = o->getProperty ("title").toString();
    const auto setting = o->getProperty ("setting").toString();
    const auto detail  = o->getProperty ("detail").toString();
    const auto notes   = o->getProperty ("notes").toString();
    const auto role    = o->getProperty ("role").toString();
    const auto ratio   = o->getProperty ("ratio").toString();
    const auto action  = o->getProperty ("action").toString();
    const auto rule    = o->getProperty ("rule").toString();
    const auto daw     = o->getProperty ("daw").toString();
    const auto where   = o->getProperty ("where").toString();
    const auto gr      = o->getProperty ("gr_db");
    const auto grStr   = o->getProperty ("gr").toString();
    const auto chain   = o->getProperty ("chain").toString();
    const auto sendSrc = o->getProperty ("source_send").toString();

    juce::String label = family.isNotEmpty() ? family
                         : (plugin.isNotEmpty() ? plugin
                            : (title.isNotEmpty() ? title
                               : (rule.isNotEmpty() ? rule : fallbackLabel)));

    if (isNumber (o->getProperty ("order")))
        label = juce::String (juce::roundToInt (asNumber (o->getProperty ("order")))) + ". "
                + (title.isNotEmpty() ? title : label);

    juce::ignoreUnused (index);

    juce::StringArray valueBits;
    if (setting.isNotEmpty())
        valueBits.add (setting);
    if (detail.isNotEmpty())
        valueBits.add (detail);
    if (action.isNotEmpty())
        valueBits.add (action);
    if (chain.isNotEmpty())
        valueBits.add (chain);
    if (sendSrc.isNotEmpty())
        valueBits.add (sendSrc);
    if (ratio.isNotEmpty())
        valueBits.add (ratio);
    if (auto* grArr = gr.getArray())
        valueBits.add (formatRange (grArr, "dB GR"));
    else if (isNumber (gr))
        valueBits.add (withUnit (formatNumber (asNumber (gr)), "dB GR"));
    if (grStr.isNotEmpty())
        valueBits.add (grStr);

    juce::String hint = notes;
    if (role.isNotEmpty())
        hint = hint.isEmpty() ? role : role + " / " + hint;
    if (where.isNotEmpty())
        hint = hint.isEmpty() ? where : hint + " / " + where;
    if (daw.isNotEmpty() && daw != "any")
        hint = hint.isEmpty() ? daw : hint + " / " + daw;

    return step (label, valueBits.joinIntoString ("  |  "), hint);
}

std::vector<RecipeStep> stepsFromVar (const juce::String& key, const juce::var& v)
{
    std::vector<RecipeStep> steps;
    if (isNullish (v))
        return steps;

    const auto label = humanizeKey (key);

    if (auto* arr = v.getArray())
    {
        if (arrayIsNumericRange (arr))
        {
            steps.push_back (step (label, formatRange (arr, unitForKey (key))));
            return steps;
        }

        if (arrayOfObjects (arr))
        {
            auto* first = (*arr)[0].getDynamicObject();
            const bool looksEq = lower (key) == "eq" || lower (key).contains ("band")
                || (first != nullptr && first->hasProperty ("freq_hz")
                    && (first->hasProperty ("type") || first->hasProperty ("gain_db")));
            const bool looksRecipe = first != nullptr
                && (first->hasProperty ("plugin") || first->hasProperty ("title")
                    || first->hasProperty ("rule") || first->hasProperty ("setting")
                    || first->hasProperty ("detail") || first->hasProperty ("action")
                    || first->hasProperty ("chain"));

            if (looksEq)
            {
                int i = 1;
                for (const auto& band : *arr)
                {
                    auto* b = band.getDynamicObject();
                    juce::String type = b != nullptr ? b->getProperty ("type").toString() : "EQ";
                    type = type.replaceCharacter ('_', ' ');
                    if (type.isNotEmpty())
                        type = type.substring (0, 1).toUpperCase() + type.substring (1);

                    juce::String freqLabel = type;
                    if (b != nullptr)
                    {
                        const auto freq = b->getProperty ("freq_hz");
                        if (auto* range = freq.getArray())
                            freqLabel += " " + formatRange (range, "Hz");
                        else if (isNumber (freq))
                            freqLabel += " " + formatNumber (asNumber (freq)) + " Hz";
                    }

                    juce::String value, hint;
                    if (b != nullptr && isNumber (b->getProperty ("gain_db")))
                        value = formatSignedDb (asNumber (b->getProperty ("gain_db")));
                    if (b != nullptr && isNumber (b->getProperty ("q")))
                        hint = "Q " + formatNumber (asNumber (b->getProperty ("q")));
                    const auto notes = b != nullptr ? b->getProperty ("notes").toString() : juce::String{};
                    if (value.isEmpty() && notes.isNotEmpty())
                        value = notes;
                    else if (notes.isNotEmpty())
                        hint = hint.isEmpty() ? notes : hint + " " + notes;

                    steps.push_back (step (freqLabel.isNotEmpty() ? freqLabel : ("EQ " + juce::String (i)),
                                           value, hint));
                    ++i;
                }
                return steps;
            }

            if (looksRecipe)
            {
                int i = 1;
                for (const auto& item : *arr)
                {
                    steps.push_back (stepFromRecord (item, i, label + " " + juce::String (i)));
                    ++i;
                }
                return steps;
            }
        }

        if (arr->size() > 0 && (*arr)[0].isString())
        {
            int i = 1;
            for (const auto& item : *arr)
            {
                steps.push_back (step (juce::String (i) + ".", item.toString()));
                ++i;
            }
            return steps;
        }

        juce::StringArray bits;
        for (const auto& item : *arr)
        {
            if (item.isObject())
                bits.add (formatEqBand (item));
            else
                bits.add (formatScalar (item, key));
        }
        steps.push_back (step (label, bits.joinIntoString (", ")));
        return steps;
    }

    if (auto* child = v.getDynamicObject())
    {
        const auto model = child->getProperty ("model").toString();
        const auto plugin = child->getProperty ("plugin").toString();
        const auto gr = child->getProperty ("gr_db");
        if (model.isNotEmpty() || plugin.isNotEmpty() || ! isNullish (gr))
        {
            juce::String value;
            if (auto* grArr = gr.getArray())
                value = formatRange (grArr, "dB GR");
            else if (isNumber (gr))
                value = withUnit (formatNumber (asNumber (gr)), "dB GR");

            const auto setting = child->getProperty ("setting").toString();
            if (value.isEmpty() && setting.isNotEmpty())
                value = setting;

            const auto hintPlugin = plugin.isNotEmpty() ? plugin : model;
            auto hint = child->getProperty ("notes").toString();
            if (hint.isEmpty())
                hint = child->getProperty ("note").toString();
            if (hint.isEmpty())
                hint = child->getProperty ("role").toString();

            const auto rowLabel = hintPlugin.isNotEmpty() ? hintPlugin : label;
            steps.push_back (step (rowLabel, value.isNotEmpty() ? value : hintPlugin,
                                   value.isNotEmpty() ? hint : hintPlugin));
            return steps;
        }

        if (child->hasProperty ("note") || child->hasProperty ("filtered"))
        {
            juce::String value = child->getProperty ("note").toString();
            juce::String hint;
            if (child->getProperty ("filtered").isBool() && static_cast<bool> (child->getProperty ("filtered")))
                hint = utf8 ("Filtr\xc3\xa9");
            steps.push_back (step (label, value, hint));
            return steps;
        }

        if (child->hasProperty ("freq_hz"))
        {
            steps.push_back (step (label, formatEqBand (v)));
            return steps;
        }

        auto nested = stepsFromObject (*child);
        if (nested.size() == 1 && nested[0].label == label)
        {
            steps.push_back (nested[0]);
        }
        else
        {
            for (auto& s : nested)
            {
                if (! s.label.startsWith (label))
                    s.label = label + utf8 (" \xc2\xb7 ") + s.label;
                steps.push_back (std::move (s));
            }
        }
        return steps;
    }

    steps.push_back (step (label, formatScalar (v, key)));
    return steps;
}

bool isInlineRecipeKey (const juce::String& key)
{
    const auto k = lower (key);
    return k == "steps" || k == "chain_steps" || k.contains ("band")
        || k == "eq" || k == "comp" || k == "plugins" || k == "order"
        || k == "chain" || k == "families" || k == "sends";
}

bool shouldSplitIntoCards (juce::DynamicObject& obj)
{
    int complex = 0, simple = 0;
    for (const auto& p : obj.getProperties())
    {
        if (skipAsStep (p.name.toString()))
            continue;

        if (isInlineRecipeKey (p.name.toString()))
        {
            ++simple;
            continue;
        }

        const bool c = p.value.isObject()
                       || (p.value.getArray() != nullptr && arrayOfObjects (p.value.getArray())
                           && ! arrayIsNumericRange (p.value.getArray()));
        if (c)
            ++complex;
        else
            ++simple;
    }
    return complex >= 1 && simple == 0;
}

juce::String cardTitleFrom (const juce::String& fallback, const juce::var& v)
{
    if (auto* obj = v.getDynamicObject())
    {
        for (const char* key : { "name", "label", "title", "problem", "style", "family", "id" })
        {
            const auto n = obj->getProperty (key).toString();
            if (n.isNotEmpty())
                return n;
        }
    }
    return fallback;
}

juce::String subtitleFromRecord (juce::DynamicObject& obj)
{
    juce::StringArray bits;
    for (const char* key : { "genre", "track_type", "daw", "family", "role", "fiche" })
    {
        const auto n = obj.getProperty (key).toString();
        if (n.isNotEmpty())
            bits.add (n);
    }
    return bits.joinIntoString (utf8 ("  \xc2\xb7  "));
}

juce::String idFromRecord (const juce::var& v, int index)
{
    if (auto* obj = v.getDynamicObject())
    {
        const auto id = obj->getProperty ("id").toString();
        if (id.isNotEmpty())
            return id;
    }
    return juce::String (index);
}

} // namespace

juce::String RecipeStep::haystack() const
{
    return (label + " " + value + " " + hint).toLowerCase();
}

juce::String RecipeCard::haystack() const
{
    juce::String h = (id + " " + title + " " + subtitle + " " + notes).toLowerCase();
    for (const auto& s : steps)
        h += " " + s.haystack();
    return h;
}

bool RecipeCard::matches (const juce::String& query) const
{
    const auto q = query.trim().toLowerCase();
    if (q.isEmpty())
        return true;
    return haystack().contains (q);
}

juce::String RecipeCard::toPlainText() const
{
    juce::String t;
    t += title;
    if (subtitle.isNotEmpty())
        t += "\n" + subtitle;
    t += "\n";

    for (const auto& s : steps)
    {
        t += "  - " + s.label;
        if (s.value.isNotEmpty())
            t += ": " + s.value;
        if (s.hint.isNotEmpty())
            t += " (" + s.hint + ")";
        t += "\n";
    }

    if (notes.isNotEmpty())
        t += "  Notes: " + notes + "\n";

    return t;
}

std::vector<RecipeCard> KnowledgeSection::filtered (const juce::String& query) const
{
    const auto q = query.trim();
    if (q.isEmpty() || title.containsIgnoreCase (q) || id.containsIgnoreCase (q))
        return cards;

    std::vector<RecipeCard> out;
    for (const auto& c : cards)
        if (c.matches (q))
            out.push_back (c);
    return out;
}

bool KnowledgeSection::matches (const juce::String& query) const
{
    return ! filtered (query).empty();
}

MixKnowledge MixKnowledge::loadEmbedded()
{
    const auto text = juce::String::fromUTF8 (BinaryData::mixknowledge_json,
                                              BinaryData::mixknowledge_jsonSize);
    return parse (text);
}

MixKnowledge MixKnowledge::parse (const juce::String& jsonText)
{
    MixKnowledge kb;
    auto parsed = juce::JSON::parse (jsonText);

    if (parsed.isVoid())
    {
        kb.error = "Impossible de parser mix-knowledge.json";
        return kb;
    }

    kb.buildFromRoot (parsed);
    kb.valid = ! kb.sections.empty();
    if (! kb.valid)
        kb.error = "Aucune section UI dans le knowledge pack";
    return kb;
}

void MixKnowledge::buildFromRoot (const juce::var& root)
{
    source = root.getProperty ("source", {}).toString();
    knowledgeVersion = root.getProperty ("version", {}).toString();

    if (auto* studio = root.getProperty ("studio", {}).getDynamicObject())
    {
        const auto n = studio->getProperty ("name").toString();
        if (n.isNotEmpty())
            studioName = n;
    }

    const auto ui = root.getProperty ("ui_sections", {});
    if (auto* arr = ui.getArray())
    {
        for (const auto& spec : *arr)
            sections.push_back (buildSection (spec, root));
    }

    std::sort (sections.begin(), sections.end(),
               [] (const KnowledgeSection& a, const KnowledgeSection& b)
               {
                   if (a.priority != b.priority)
                       return a.priority < b.priority;
                   return a.title < b.title;
               });
}

KnowledgeSection MixKnowledge::buildSection (const juce::var& spec, const juce::var& root) const
{
    KnowledgeSection section;
    if (auto* obj = spec.getDynamicObject())
    {
        section.id = obj->getProperty ("id").toString();
        section.title = obj->getProperty ("title").toString();
        if (isNumber (obj->getProperty ("priority")))
            section.priority = juce::roundToInt (asNumber (obj->getProperty ("priority")));
    }
    else if (spec.isString())
    {
        section.id = spec.toString();
        section.title = humanizeKey (section.id);
    }

    if (section.title.isEmpty())
        section.title = humanizeKey (section.id);

    section.cards = cardsForSectionId (section.id, root);

    if (section.cards.empty())
    {
        RecipeCard empty;
        empty.id = section.id + ".empty";
        empty.title = section.title;
        empty.notes = utf8 ("Pas encore de contenu pour cette section. Ajoutez des champs dans mix-knowledge.json.");
        section.cards.push_back (std::move (empty));
    }

    return section;
}

std::vector<RecipeCard> MixKnowledge::genericCards (const juce::String& title,
                                                    const juce::String& id,
                                                    const juce::var& value) const
{
    std::vector<RecipeCard> out;

    if (isNullish (value))
        return out;

    if (auto* arr = value.getArray())
    {
        if (arrayOfObjects (arr))
        {
            int i = 0;
            for (const auto& item : *arr)
            {
                const auto childTitle = cardTitleFrom (title + " " + juce::String (i + 1), item);
                const auto childId = id + "." + idFromRecord (item, i);
                auto nested = genericCards (childTitle, childId, item);
                out.insert (out.end(), nested.begin(), nested.end());
                ++i;
            }
            return out;
        }

        RecipeCard card;
        card.id = id;
        card.title = title;
        card.steps = stepsFromVar (title, value);
        out.push_back (std::move (card));
        return out;
    }

    if (auto* obj = value.getDynamicObject())
    {
        const auto split = shouldSplitIntoCards (*obj);

        if (split)
        {
            RecipeCard leftovers;
            leftovers.id = id + ".overview";
            leftovers.title = title;

            for (const auto& p : obj->getProperties())
            {
                const auto key = p.name.toString();
                if (skipAsStep (key))
                    continue;

                if (isInlineRecipeKey (key))
                {
                    auto st = stepsFromVar (key, p.value);
                    leftovers.steps.insert (leftovers.steps.end(), st.begin(), st.end());
                    continue;
                }

                const bool childComplex = p.value.isObject()
                    || (p.value.getArray() != nullptr && arrayOfObjects (p.value.getArray())
                        && ! arrayIsNumericRange (p.value.getArray()));

                if (childComplex)
                {
                    auto nested = genericCards (humanizeKey (key), id + "." + key, p.value);
                    out.insert (out.end(), nested.begin(), nested.end());
                }
                else
                {
                    auto st = stepsFromVar (key, p.value);
                    leftovers.steps.insert (leftovers.steps.end(), st.begin(), st.end());
                }
            }

            leftovers.notes = obj->getProperty ("notes").toString();

            if (! leftovers.steps.empty() || leftovers.notes.isNotEmpty())
                out.insert (out.begin(), leftovers);

            return out;
        }

        RecipeCard card;
        card.id = id;
        card.title = cardTitleFrom (title, value);
        card.subtitle = subtitleFromRecord (*obj);
        card.steps = stepsFromObject (*obj);
        card.notes = obj->getProperty ("notes").toString();
        if (card.notes.isEmpty())
            card.notes = obj->getProperty ("interdit").toString();
        out.push_back (std::move (card));
        return out;
    }

    RecipeCard card;
    card.id = id;
    card.title = title;
    card.steps = stepsFromVar (title, value);
    // Don't prefix with humanized title twice — stepsFromVar already labelled.
    if (value.getArray() != nullptr && value.getArray()->size() > 0 && (*value.getArray())[0].isString())
    {
        // numbered list already uses "1." labels
    }
    out.push_back (std::move (card));
    return out;
}

std::vector<RecipeCard> MixKnowledge::cardsForSectionId (const juce::String& id, const juce::var& root) const
{
    const auto studio = root.getProperty ("studio", {});
    const auto presets = root.getProperty ("presets", {});
    const auto drums = root.getProperty ("drums", {});

    auto collectArrayOrMap = [this] (const juce::var& node, const juce::String& prefix) -> std::vector<RecipeCard>
    {
        std::vector<RecipeCard> out;
        if (auto* arr = node.getArray())
        {
            int i = 0;
            for (const auto& item : *arr)
            {
                auto more = genericCards (cardTitleFrom (prefix, item),
                                          prefix + "." + idFromRecord (item, i),
                                          item);
                out.insert (out.end(), more.begin(), more.end());
                ++i;
            }
            return out;
        }

        if (auto* obj = node.getDynamicObject())
        {
            for (const auto& p : obj->getProperties())
            {
                auto more = genericCards (humanizeKey (p.name.toString()),
                                          prefix + "." + p.name.toString(),
                                          p.value);
                out.insert (out.end(), more.begin(), more.end());
            }
        }
        return out;
    };

    auto append = [] (std::vector<RecipeCard>& dest, std::vector<RecipeCard> more)
    {
        dest.insert (dest.end(), more.begin(), more.end());
    };

    if (id == "session_checklist")
    {
        auto cards = collectArrayOrMap (root.getProperty ("checklists", {}), "checklists");

        RecipeCard workflow;
        workflow.id = "session.workflow";
        workflow.title = "Workflow session";
        workflow.subtitle = studioName + utf8 (" \xe2\x80\x94 Alger");
        if (auto* w = studio.getProperty ("workflow", {}).getDynamicObject())
        {
            workflow.steps = stepsFromObject (*w);
            workflow.notes = w->getProperty ("notes").toString();
        }
        cards.push_back (std::move (workflow));

        RecipeCard rec;
        rec.id = "session.recording";
        rec.title = utf8 ("Enregistrement");
        if (auto* r = studio.getProperty ("recording", {}).getDynamicObject())
            rec.steps = stepsFromObject (*r);
        cards.push_back (std::move (rec));

        RecipeCard chain;
        chain.id = "session.vocal_chain";
        chain.title = utf8 ("Ordre de cha\xc3\xaene voix");
        chain.notes = utf8 ("Ordre d'inserts \xe2\x80\x94 appliquer manuellement dans FL Studio / Studio One.");
        chain.steps = stepsFromVar ("vocal_chain_order", root.getProperty ("vocal_chain_order", {}));
        cards.push_back (std::move (chain));

        RecipeCard mix;
        mix.id = "session.mix_reminders";
        mix.title = "Rappels mix";
        if (auto* g = studio.getProperty ("gain_staging", {}).getDynamicObject())
            mix.steps = stepsFromObject (*g);
        cards.push_back (std::move (mix));

        return cards;
    }

    if (id == "vocal_rap")
    {
        auto cards = collectArrayOrMap (presets, "presets");
        auto tune = genericCards ("Auto-Tune", "autotune_guide", root.getProperty ("autotune_guide", {}));
        auto sends = genericCards ("Sends", "sends_guide", root.getProperty ("sends_guide", {}));
        RecipeCard chain;
        chain.id = "vocal.chain_order";
        chain.title = utf8 ("Ordre de cha\xc3\xaene");
        chain.steps = stepsFromVar ("vocal_chain_order", root.getProperty ("vocal_chain_order", {}));
        cards.insert (cards.begin(), std::move (chain));
        append (cards, std::move (tune));
        append (cards, std::move (sends));
        return cards;
    }

    if (id == "eq_guide")
    {
        std::vector<RecipeCard> cards;
        append (cards, genericCards (utf8 ("HPF d\xc3\xa9" "fauts"), "hpf_defaults",
                                     root.getProperty ("hpf_defaults", {})));
        append (cards, collectArrayOrMap (root.getProperty ("eq_recipes_by_style", {}), "eq_recipes"));
        append (cards, collectArrayOrMap (root.getProperty ("eq_guide", {}), "eq_guide"));
        return cards;
    }

    if (id == "drums_808")
    {
        auto cards = genericCards ("Drums", "drums", drums);
        append (cards, genericCards ("808", "eight_oh_eight", root.getProperty ("eight_oh_eight", {})));
        append (cards, genericCards ("Compression", "compression_guide",
                                     root.getProperty ("compression_guide", {})));
        return cards;
    }

    if (id == "gain_staging")
    {
        std::vector<RecipeCard> cards;
        append (cards, genericCards ("Gain staging", "gain_staging",
                                     studio.getProperty ("gain_staging", {})));
        append (cards, genericCards (utf8 ("Niveaux d'entr\xc3\xa9" "e"), "recording",
                                     studio.getProperty ("recording", {})));
        append (cards, collectArrayOrMap (root.getProperty ("pipeline_rules", {}), "pipeline"));
        return cards;
    }

    if (id == "mastering")
        return genericCards ("Mastering cible", "mastering", root.getProperty ("mastering", {}));

    if (id == "fl_recording")
    {
        std::vector<RecipeCard> cards;
        if (auto* arr = root.getProperty ("checklists", {}).getArray())
        {
            int i = 0;
            for (const auto& item : *arr)
            {
                auto* o = item.getDynamicObject();
                const auto daw = o != nullptr ? o->getProperty ("daw").toString() : juce::String{};
                const auto cid = o != nullptr ? o->getProperty ("id").toString() : juce::String{};
                const auto title = o != nullptr ? o->getProperty ("title").toString() : juce::String{};
                const bool fl = daw.containsIgnoreCase ("FL") || cid.containsIgnoreCase ("fl")
                                || title.containsIgnoreCase ("FL");
                if (fl)
                    append (cards, genericCards (cardTitleFrom ("FL", item),
                                                 "fl_recording." + idFromRecord (item, i), item));
                ++i;
            }
        }
        append (cards, genericCards (utf8 ("Enregistrement"), "recording",
                                     studio.getProperty ("recording", {})));
        return cards;
    }

    if (id == "parallel_buses")
        return collectArrayOrMap (root.getProperty ("parallel_buses", {}), "parallel_buses");

    if (id == "plugin_stacks")
    {
        auto cards = collectArrayOrMap (root.getProperty ("plugin_stacks", {}), "plugin_stacks");
        append (cards, genericCards (utf8 ("Stack c\xc5\x93ur"), "core_plugin_stack",
                                     studio.getProperty ("core_plugin_stack", {})));
        return cards;
    }

    // Optional explicit bindings: { "section_bindings": { "id": ["path", "path"] } }
    if (auto* bindings = root.getProperty ("section_bindings", {}).getDynamicObject())
    {
        const auto paths = bindings->getProperty (id);
        if (auto* arr = paths.getArray())
        {
            std::vector<RecipeCard> cards;
            for (const auto& pathVar : *arr)
            {
                const auto path = pathVar.toString();
                juce::var node = root;
                for (auto token : juce::StringArray::fromTokens (path, "/", {}))
                {
                    if (token.isEmpty())
                        continue;
                    node = node.getProperty (juce::Identifier (token), {});
                }
                auto leaf = path.fromLastOccurrenceOf ("/", false, false);
                if (leaf.isEmpty())
                    leaf = path;
                auto more = genericCards (humanizeKey (leaf), path, node);
                cards.insert (cards.end(), more.begin(), more.end());
            }
            if (! cards.empty())
                return cards;
        }
    }

    // Generic: top-level key matching the section id
    if (auto* rootObj = root.getDynamicObject())
    {
        if (rootObj->hasProperty (juce::Identifier (id)))
            return genericCards (humanizeKey (id), id, rootObj->getProperty (id));

        // Nested "studio/<id>" etc.
        for (const auto& p : rootObj->getProperties())
        {
            if (auto* child = p.value.getDynamicObject())
            {
                if (child->hasProperty (juce::Identifier (id)))
                    return genericCards (humanizeKey (id), p.name.toString() + "." + id,
                                         child->getProperty (id));
            }
        }
    }

    return {};
}

std::vector<KnowledgeSection> MixKnowledge::visibleSections (const juce::String& query) const
{
    std::vector<KnowledgeSection> out;
    for (const auto& s : sections)
        if (s.matches (query))
            out.push_back (s);
    return out;
}

const KnowledgeSection* MixKnowledge::findSection (const juce::String& id) const
{
    for (const auto& s : sections)
        if (s.id == id)
            return &s;
    return nullptr;
}

juce::String MixKnowledge::formatClipboard (const juce::String& sectionId,
                                            const juce::String& query) const
{
    const auto* section = findSection (sectionId);
    juce::String t;
    t += "K3CH MIX ASSIST\n";
    t += studioName + juce::String::fromUTF8 (" \xe2\x80\x94 Alger\n");
    if (knowledgeVersion.isNotEmpty())
        t += "Knowledge " + knowledgeVersion + "\n";
    t += juce::String::repeatedString ("=", 42) + "\n\n";

    if (section == nullptr)
    {
        t += "(section introuvable)\n";
        return t;
    }

    t += section->title + "\n\n";

    for (const auto& card : section->filtered (query))
    {
        t += card.toPlainText();
        t += "\n";
    }

    t += juce::String::repeatedString ("-", 42) + "\n";
    t += "Recette a appliquer manuellement dans FL Studio / Studio One.\n";
    t += utf8 ("Points de depart \xe2\x80\x94 adapter a la source.\n");
    return t;
}

juce::String MixKnowledge::aboutText() const
{
    juce::String t;
    t += "K3CH Mix Assist " + juce::String (K3CH_VERSION) + "\n";
    t += studioName + juce::String::fromUTF8 (" \xe2\x80\x94 Alger\n\n");
    t += "Insert utilitaire : bypass / pass-through, recipes locales.\n";
    t += utf8 ("Aucune API r\xc3\xa9seau, aucun mod\xc3\xa8le distant.\n\n");
    if (knowledgeVersion.isNotEmpty())
        t += "Knowledge pack : " + knowledgeVersion + "\n";
    if (source.isNotEmpty())
        t += "Source : " + source + "\n";
    t += "\nLes chiffres sont des points de depart de studio,\npas une loi de mastering.";
    return t;
}

} // namespace k3ch
