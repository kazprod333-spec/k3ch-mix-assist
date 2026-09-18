#include "CommandParser.h"

#include <cctype>

namespace k3ch_presets
{
namespace
{

juce::String foldCmd (juce::String s)
{
    s = s.toLowerCase().trim();
    s = s.replace (juce::String::charToString (juce::juce_wchar (0x00AB)), " ");
    s = s.replace (juce::String::charToString (juce::juce_wchar (0x00BB)), " ");
    s = s.replaceCharacters (".,;:!?\"'", "        ");
    const juce::juce_wchar acc[] = {
        0x00E9, 0x00E8, 0x00EA, 0x00EB, 0x00E0, 0x00E2, 0x00E7, 0x00F4, 0x00F9, 0x00FB, 0x00EE, 0x00EF, 0
    };
    const char* repl = "eeeeaecouuii";
    for (int i = 0; acc[i] != 0; ++i)
        s = s.replace (juce::String::charToString (acc[i]), juce::String::charToString ((juce::juce_wchar) repl[i]));
    while (s.contains ("  "))
        s = s.replace ("  ", " ");
    return s.trim();
}

int parseSendIndex (const juce::String& f)
{
    if (f.contains ("send b") || f.contains ("sendb") || f.contains ("sur b")
        || f.contains ("on b") || f.endsWith (" b") || f.contains (" send 2")
        || f.contains ("sur 2") || f.contains ("slot b"))
        return 1;
    if (f.contains ("send a") || f.contains ("senda") || f.contains ("sur a")
        || f.contains ("on a") || f.endsWith (" a") || f.contains (" send 1")
        || f.contains ("sur 1") || f.contains ("slot a"))
        return 0;
    return -1;
}

bool hasLoadVerb (const juce::String& f)
{
    return f.contains ("charge") || f.contains ("load") || f.contains ("applique")
        || f.contains ("apply") || f.contains ("prend") || f.contains ("take")
        || f.contains ("use") || f.contains ("utilise") || f.contains ("mets")
        || f.contains ("met ") || f.startsWith ("met ") || f.contains ("put")
        || f.contains ("assign") || f.contains ("envoie") || f.contains ("send ");
}

} // namespace

CommandParser::CommandParser (const PresetLibrary& lib)
    : library (lib)
{
}

Command CommandParser::parse (const juce::String& text) const
{
    Command cmd;
    const auto raw = text.trim();
    const auto f = foldCmd (raw);

    if (f.isEmpty())
    {
        cmd.message = juce::String::fromUTF8 ("Tape une commande : \xc2\xab charge ModernRap \xc2\xbb, \xc2\xab mets Air sur send A \xc2\xbb");
        return cmd;
    }

    if (f == "help" || f == "aide" || f == "?")
    {
        cmd.type = Command::Type::Help;
        cmd.ok = true;
        cmd.message = juce::String::fromUTF8 (
            "Exemples : charge ModernRap  ·  mets Air parall\xc3\xa8le sur send A  ·  harsh sur send B  ·  send A -15  ·  dry 80  ·  bypass");
        return cmd;
    }

    if (f == "bypass" || f == "coupe" || f == "mute dsp")
    {
        cmd.type = Command::Type::Bypass;
        cmd.bypassOn = true;
        cmd.ok = true;
        cmd.message = "Bypass ON";
        return cmd;
    }
    if (f == "unbypass" || f.contains ("bypass off") || f == "active" || f == "engage")
    {
        cmd.type = Command::Type::Bypass;
        cmd.bypassOn = false;
        cmd.ok = true;
        cmd.message = "Bypass OFF";
        return cmd;
    }

    // dry/wet 0–100 or 0–1
    if (f.startsWith ("dry") || f.startsWith ("wet") || f.startsWith ("drywet") || f.startsWith ("mix"))
    {
        auto bits = juce::StringArray::fromTokens (f, " ", {});
        float v = 1.0f;
        bool found = false;
        for (auto& b : bits)
        {
            auto t = b.replace ("%", "").replace ("pct", "");
            if (t.containsOnly ("0123456789.+-"))
            {
                v = t.getFloatValue();
                found = true;
                break;
            }
        }
        if (found)
        {
            if (v > 1.0f)
                v *= 0.01f;
            v = juce::jlimit (0.0f, 1.0f, v);
            if (f.startsWith ("dry") && ! f.startsWith ("drywet") && ! f.contains ("wet"))
                v = 1.0f - v;
            cmd.type = Command::Type::SetDryWet;
            cmd.value = v;
            cmd.ok = true;
            cmd.message = "Dry/Wet " + juce::String (juce::roundToInt (v * 100.0f)) + " %";
            return cmd;
        }
    }

    const int sendIdx = parseSendIndex (f);

    // send A -15 / niveau send b -12db
    {
        auto bits = juce::StringArray::fromTokens (f, " ", {});
        for (int i = 0; i < bits.size(); ++i)
        {
            auto t = bits[i].replace ("db", "").replace ("dB", "");
            if (t.startsWith ("-") && t.substring (1).containsOnly ("0123456789."))
            {
                const float db = juce::jlimit (-60.0f, 0.0f, t.getFloatValue());
                if (sendIdx >= 0 || f.contains ("niveau") || f.contains ("level") || f.contains ("send"))
                {
                    cmd.type = Command::Type::SetSendLevel;
                    cmd.sendIndex = sendIdx >= 0 ? sendIdx : 0;
                    cmd.value = db;
                    cmd.ok = true;
                    cmd.message = juce::String ("Send ") + (cmd.sendIndex == 0 ? "A" : "B")
                                  + " " + juce::String (db, 1) + " dB";
                    return cmd;
                }
            }
        }
    }

    const int fx = library.findFxIndex (raw.isNotEmpty() ? raw : f);
    const int vocal = library.findVocalIndex (raw.isNotEmpty() ? raw : f);

    const bool sendish = sendIdx >= 0
                         || f.contains ("send")
                         || f.contains ("mets")
                         || f.contains ("met ")
                         || f.startsWith ("met")
                         || f.contains ("envoie")
                         || f.contains ("sur a")
                         || f.contains ("sur b");

    if (fx >= 0 && (sendish || (vocal < 0 && (hasLoadVerb (f) || sendIdx >= 0 || f.contains ("air")
                                              || f.contains ("harsh") || f.contains ("plate")
                                              || f.contains ("delay") || f.contains ("body")
                                              || f.contains ("slap") || f.contains ("hall")
                                              || f.contains ("room") || f.contains ("sat")))))
    {
        // If both match, prefer FX when a send is mentioned.
        if (sendish || sendIdx >= 0 || vocal < 0)
        {
            const auto* p = library.fxAt (fx);
            cmd.type = Command::Type::LoadSend;
            cmd.fxIndex = fx;
            cmd.sendIndex = sendIdx >= 0 ? sendIdx : 0;
            cmd.presetId = p != nullptr ? p->id : juce::String{};
            cmd.ok = true;
            cmd.message = juce::String ("Send ") + (cmd.sendIndex == 0 ? "A" : "B") + juce::String::fromUTF8 (" \xe2\x86\x90 ")
                          + (p != nullptr ? p->name : "?");
            return cmd;
        }
    }

    if (vocal >= 0)
    {
        const auto* p = library.vocalAt (vocal);
        cmd.type = Command::Type::LoadVocal;
        cmd.vocalIndex = vocal;
        cmd.presetId = p != nullptr ? p->id : juce::String{};
        cmd.ok = true;
        cmd.message = juce::String::fromUTF8 ("Cha\xc3\xaene voix : ") + (p != nullptr ? p->name : "?");
        return cmd;
    }

    if (fx >= 0)
    {
        const auto* p = library.fxAt (fx);
        cmd.type = Command::Type::LoadSend;
        cmd.fxIndex = fx;
        cmd.sendIndex = sendIdx >= 0 ? sendIdx : 0;
        cmd.presetId = p != nullptr ? p->id : juce::String{};
        cmd.ok = true;
        cmd.message = juce::String ("Send ") + (cmd.sendIndex == 0 ? "A" : "B") + juce::String::fromUTF8 (" \xe2\x86\x90 ")
                      + (p != nullptr ? p->name : "?");
        return cmd;
    }

    cmd.message = juce::String::fromUTF8 ("Commande inconnue \xc2\xab ") + raw + juce::String::fromUTF8 (" \xc2\xbb \xe2\x80\x94 tape aide");
    return cmd;
}

} // namespace k3ch_presets
