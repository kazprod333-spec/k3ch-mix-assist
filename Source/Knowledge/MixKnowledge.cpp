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
        || key == "docs_pending_ingest" || key == "source" || key == "version";
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
    if (k.contains ("amount") || k.contains ("tune"))
        return "%";

    return {};
}

juce::String humanizeKey (juce::String key)
{
    static const std::map<juce::String, juce::String> known {
        { "hpf_hz", "HPF" },
        { "eq", "EQ" },
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
        { "recording", "Enregistrement" },
        { "gain_staging", "Gain staging" },
        { "vocal_chain_order", "Ordre de chaîne voix" },
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

    juce::String t = type.isNotEmpty()
                         ? (type.substring (0, 1).toUpperCase() + type.substring (1))
                         : "EQ";

    if (isNumber (freq))
        t += " " + formatNumber (asNumber (freq)) + " Hz";

    juce::String value;
    if (isNumber (gain))
        value = formatSignedDb (asNumber (gain));

    juce::String hint;
    if (isNumber (q))
        hint = "Q " + formatNumber (asNumber (q));

    if (hint.isNotEmpty() && value.isNotEmpty())
        return t + "  ·  " + value + "  ·  " + hint;
    if (value.isNotEmpty())
        return t + "  ·  " + value;
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
        if (isMetaKey (key) || key == "name")
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

        if (arrayOfObjects (arr) && (lower (key) == "eq" || lower (key).contains ("band")))
        {
            int i = 1;
            for (const auto& band : *arr)
            {
                auto* b = band.getDynamicObject();
                juce::String type = b != nullptr ? b->getProperty ("type").toString() : "EQ";
                if (type.isNotEmpty())
                    type = type.substring (0, 1).toUpperCase() + type.substring (1);

                juce::String freqLabel = type;
                if (b != nullptr && isNumber (b->getProperty ("freq_hz")))
                    freqLabel += " " + formatNumber (asNumber (b->getProperty ("freq_hz"))) + " Hz";

                juce::String value, hint;
                if (b != nullptr && isNumber (b->getProperty ("gain_db")))
                    value = formatSignedDb (asNumber (b->getProperty ("gain_db")));
                if (b != nullptr && isNumber (b->getProperty ("q")))
                    hint = "Q " + formatNumber (asNumber (b->getProperty ("q")));

                steps.push_back (step (freqLabel.isNotEmpty() ? freqLabel : ("EQ " + juce::String (i)),
                                       value, hint));
                ++i;
            }
            return steps;
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
        const auto gr = child->getProperty ("gr_db");
        if (model.isNotEmpty() || ! isNullish (gr))
        {
            juce::String value;
            if (auto* grArr = gr.getArray())
                value = formatRange (grArr, "dB GR");
            else if (isNumber (gr))
                value = withUnit (formatNumber (asNumber (gr)), "dB GR");

            const auto hint = child->getProperty ("note").toString();
            steps.push_back (step (label, value.isNotEmpty() ? value : model,
                                   value.isNotEmpty() ? model : hint));
            return steps;
        }

        if (child->hasProperty ("note") || child->hasProperty ("filtered"))
        {
            juce::String value = child->getProperty ("note").toString();
            juce::String hint;
            if (child->getProperty ("filtered").isBool() && static_cast<bool> (child->getProperty ("filtered")))
                hint = "Filtré";
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
                    s.label = label + " · " + s.label;
                steps.push_back (std::move (s));
            }
        }
        return steps;
    }

    steps.push_back (step (label, formatScalar (v, key)));
    return steps;
}

int countComplexChildren (juce::DynamicObject& obj)
{
    int complex = 0, simple = 0;
    for (const auto& p : obj.getProperties())
    {
        if (isMetaKey (p.name.toString()) || p.name.toString() == "name")
            continue;

        const bool c = p.value.isObject()
                       || (p.value.getArray() != nullptr && arrayOfObjects (p.value.getArray())
                           && ! arrayIsNumericRange (p.value.getArray()));
        if (c)
            ++complex;
        else
            ++simple;
    }
    juce::ignoreUnused (simple);
    return complex;
}

juce::String cardTitleFrom (const juce::String& fallback, const juce::var& v)
{
    if (auto* obj = v.getDynamicObject())
    {
        const auto n = obj->getProperty ("name").toString();
        if (n.isNotEmpty())
            return n;
    }
    return fallback;
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
        empty.notes = "Pas encore de contenu pour cette section. Ajoutez des champs dans mix-knowledge.json.";
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

    if (auto* obj = value.getDynamicObject())
    {
        const auto complex = countComplexChildren (*obj);

        if (complex >= 1)
        {
            RecipeCard leftovers;
            leftovers.id = id + ".overview";
            leftovers.title = title;

            for (const auto& p : obj->getProperties())
            {
                const auto key = p.name.toString();
                if (isMetaKey (key) || key == "name")
                    continue;

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

            if (! leftovers.steps.empty())
                out.insert (out.begin(), leftovers);

            return out;
        }

        RecipeCard card;
        card.id = id;
        card.title = cardTitleFrom (title, value);
        card.steps = stepsFromObject (*obj);
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

    auto allPresetsAsCards = [this, &presets] (bool eqOnly) -> std::vector<RecipeCard>
    {
        std::vector<RecipeCard> out;
        auto* obj = presets.getDynamicObject();
        if (obj == nullptr)
            return out;

        for (const auto& p : obj->getProperties())
        {
            auto cards = genericCards (humanizeKey (p.name.toString()),
                                       "presets." + p.name.toString(),
                                       p.value);

            if (eqOnly)
            {
                for (auto& c : cards)
                {
                    std::vector<RecipeStep> eqSteps;
                    for (const auto& s : c.steps)
                    {
                        const auto h = s.haystack();
                        if (h.contains ("hz") || h.contains ("eq") || h.contains ("hpf")
                            || h.contains ("bell") || h.contains ("shelf") || s.value.contains ("dB"))
                        {
                            if (h.contains ("gr") || h.contains ("comp") || h.contains ("reverb")
                                || h.contains ("delay") || h.contains ("tune") || h.contains ("de-ess"))
                                continue;
                            eqSteps.push_back (s);
                        }
                    }
                    c.steps.swap (eqSteps);
                    if (! c.steps.empty())
                        out.push_back (std::move (c));
                }
            }
            else
            {
                out.insert (out.end(), cards.begin(), cards.end());
            }
        }
        return out;
    };

    if (id == "session_checklist")
    {
        std::vector<RecipeCard> cards;

        RecipeCard workflow;
        workflow.id = "session.workflow";
        workflow.title = "Workflow session";
        workflow.subtitle = studioName + " — Alger";
        if (auto* w = studio.getProperty ("workflow", {}).getDynamicObject())
        {
            workflow.steps = stepsFromObject (*w);
        }
        cards.push_back (std::move (workflow));

        RecipeCard rec;
        rec.id = "session.recording";
        rec.title = "Enregistrement";
        if (auto* r = studio.getProperty ("recording", {}).getDynamicObject())
            rec.steps = stepsFromObject (*r);
        cards.push_back (std::move (rec));

        RecipeCard chain;
        chain.id = "session.vocal_chain";
        chain.title = "Ordre de chaîne voix";
        chain.notes = "Ordre d'inserts — appliquer manuellement dans FL Studio / Studio One.";
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
        auto cards = allPresetsAsCards (false);
        RecipeCard chain;
        chain.id = "vocal.chain_order";
        chain.title = "Ordre de chaîne";
        chain.steps = stepsFromVar ("vocal_chain_order", root.getProperty ("vocal_chain_order", {}));
        cards.insert (cards.begin(), std::move (chain));
        return cards;
    }

    if (id == "eq_guide")
        return allPresetsAsCards (true);

    if (id == "drums_808")
    {
        auto cards = genericCards ("Drums", "drums", drums);
        auto extra = genericCards ("Compression", "compression_guide",
                                   root.getProperty ("compression_guide", {}));
        cards.insert (cards.end(), extra.begin(), extra.end());
        return cards;
    }

    if (id == "gain_staging")
    {
        std::vector<RecipeCard> cards;
        auto g = genericCards ("Gain staging", "gain_staging",
                               studio.getProperty ("gain_staging", {}));
        auto r = genericCards ("Niveaux d'entrée", "recording",
                               studio.getProperty ("recording", {}));
        cards.insert (cards.end(), g.begin(), g.end());
        cards.insert (cards.end(), r.begin(), r.end());
        return cards;
    }

    if (id == "mastering")
        return genericCards ("Mastering cible", "mastering", root.getProperty ("mastering", {}));

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
    t += "Points de depart — adapter a la source.\n";
    return t;
}

juce::String MixKnowledge::aboutText() const
{
    juce::String t;
    t += "K3CH Mix Assist " + juce::String (K3CH_VERSION) + "\n";
    t += studioName + juce::String::fromUTF8 (" \xe2\x80\x94 Alger\n\n");
    t += "Insert utilitaire : bypass / pass-through, recipes locales.\n";
    t += "Aucune API reseau, aucun modele distant.\n\n";
    if (knowledgeVersion.isNotEmpty())
        t += "Knowledge pack : " + knowledgeVersion + "\n";
    if (source.isNotEmpty())
        t += "Source : " + source + "\n";
    t += "\nLes chiffres sont des points de depart de studio,\npas une loi de mastering.";
    return t;
}

} // namespace k3ch
