#include "MasterPresets.h"

#include <cstring>

namespace k3ch_master
{

namespace
{
MasterSettings streaming14()
{
    MasterSettings s;
    s.inputDb = -1.2f;
    s.lowShelfHz = 110.0f;
    s.lowShelfDb = 0.4f;
    s.highShelfHz = 9000.0f;
    s.highShelfDb = 0.7f;
    s.midOn = true;
    s.midHz = 380.0f;
    s.midGainDb = -0.4f;
    s.midQ = 0.8f;
    s.satOn = true;
    s.satDrive = 0.10f;
    s.satMix = 0.12f;
    s.width = 1.04f;
    s.bassMonoHz = 120.0f;
    s.clipOn = true;
    s.clipCeilingDb = -0.8f;
    s.limOn = true;
    s.limCeilingDb = -1.2f;
    s.limReleaseMs = 130.0f;
    s.limLookaheadMs = 3.0f;
    s.outputDb = 0.0f;
    s.tpOn = true;
    s.tpCeilingDb = -1.0f;
    return s;
}

MasterSettings clubLoud()
{
    MasterSettings s;
    s.inputDb = 2.4f;
    s.lowShelfHz = 85.0f;
    s.lowShelfDb = 1.3f;
    s.highShelfHz = 10500.0f;
    s.highShelfDb = 0.6f;
    s.midOn = true;
    s.midHz = 2200.0f;
    s.midGainDb = 0.8f;
    s.midQ = 0.85f;
    s.satOn = true;
    s.satDrive = 0.28f;
    s.satMix = 0.34f;
    s.width = 1.16f;
    s.bassMonoHz = 100.0f;
    s.clipOn = true;
    s.clipCeilingDb = -0.25f;
    s.limOn = true;
    s.limCeilingDb = -0.4f;
    s.limReleaseMs = 38.0f;
    s.limLookaheadMs = 1.8f;
    s.outputDb = 0.0f;
    s.tpOn = true;
    s.tpCeilingDb = -0.5f;
    return s;
}

MasterSettings radio()
{
    MasterSettings s;
    s.inputDb = 1.1f;
    s.lowShelfHz = 150.0f;
    s.lowShelfDb = 0.9f;
    s.highShelfHz = 7200.0f;
    s.highShelfDb = 1.3f;
    s.midOn = true;
    s.midHz = 2800.0f;
    s.midGainDb = 1.4f;
    s.midQ = 1.15f;
    s.satOn = true;
    s.satDrive = 0.18f;
    s.satMix = 0.22f;
    s.width = 1.0f;
    s.bassMonoHz = 140.0f;
    s.clipOn = true;
    s.clipCeilingDb = -0.55f;
    s.limOn = true;
    s.limCeilingDb = -1.0f;
    s.limReleaseMs = 55.0f;
    s.limLookaheadMs = 2.4f;
    s.outputDb = 0.2f;
    s.tpOn = true;
    s.tpCeilingDb = -1.0f;
    return s;
}

MasterSettings glue()
{
    MasterSettings s;
    s.inputDb = 0.0f;
    s.lowShelfHz = 130.0f;
    s.lowShelfDb = 0.25f;
    s.highShelfHz = 8500.0f;
    s.highShelfDb = 0.3f;
    s.midOn = false;
    s.midHz = 2500.0f;
    s.midGainDb = 0.0f;
    s.midQ = 1.0f;
    s.satOn = true;
    s.satDrive = 0.08f;
    s.satMix = 0.12f;
    s.width = 1.02f;
    s.bassMonoHz = 120.0f;
    s.clipOn = true;
    s.clipCeilingDb = -0.7f;
    s.limOn = true;
    s.limCeilingDb = -1.0f;
    s.limReleaseMs = 160.0f;
    s.limLookaheadMs = 3.5f;
    s.outputDb = 0.0f;
    s.tpOn = true;
    s.tpCeilingDb = -1.0f;
    return s;
}

MasterSettings trapK3ch()
{
    MasterSettings s;
    s.inputDb = 1.6f;
    s.lowShelfHz = 70.0f;
    s.lowShelfDb = 1.8f;
    s.highShelfHz = 11000.0f;
    s.highShelfDb = 1.1f;
    s.midOn = true;
    s.midHz = 400.0f;
    s.midGainDb = -0.9f;
    s.midQ = 1.0f;
    s.satOn = true;
    s.satDrive = 0.22f;
    s.satMix = 0.26f;
    s.width = 1.12f;
    s.bassMonoHz = 100.0f;
    s.clipOn = true;
    s.clipCeilingDb = -0.35f;
    s.limOn = true;
    s.limCeilingDb = -0.8f;
    s.limReleaseMs = 48.0f;
    s.limLookaheadMs = 2.0f;
    s.outputDb = 0.0f;
    s.tpOn = true;
    s.tpCeilingDb = -1.0f;
    return s;
}

MasterSettings bypassNeutral()
{
    MasterSettings s;
    s.inputDb = 0.0f;
    s.lowShelfHz = 120.0f;
    s.lowShelfDb = 0.0f;
    s.highShelfHz = 8000.0f;
    s.highShelfDb = 0.0f;
    s.midOn = false;
    s.midHz = 2500.0f;
    s.midGainDb = 0.0f;
    s.midQ = 1.0f;
    s.satOn = false;
    s.satDrive = 0.0f;
    s.satMix = 0.0f;
    s.width = 1.0f;
    s.bassMonoHz = 120.0f;
    s.clipOn = false;
    s.clipCeilingDb = -0.3f;
    s.limOn = false;
    s.limCeilingDb = -0.3f;
    s.limReleaseMs = 80.0f;
    s.limLookaheadMs = 2.0f;
    s.outputDb = 0.0f;
    s.tpOn = false;
    s.tpCeilingDb = -1.0f;
    return s;
}
} // namespace

const std::vector<MasterPreset>& allMasterPresets()
{
    static const std::vector<MasterPreset> presets {
        { "streaming_14", "Streaming -14 LUFS",
          "Doux, streaming. Plafond TP -1, glue leger, largeur discrete.",
          streaming14() },
        { "club_loud", "Club / Loud",
          "Plus fort, sat et clip assumés. Pour club / bounce social séparé.",
          clubLoud() },
        { "radio", "Radio / Broadcast",
          "Présence 2.8 k, largeur mono-safe, plafond -1 dBTP.",
          radio() },
        { "transparent_glue", "Transparent Glue",
          "Presque invisible. Sat et limiteur très légers.",
          glue() },
        { "trap_k3ch", "Hip-Hop / Trap Master",
          "Vibe Maison K3CH : 808 grave, coupe 400, air 11 k, sat.",
          trapK3ch() },
        { "bypass_neutral", "Bypass / Neutral",
          "Chaine plate. Aucun traitement (hors gain 0 dB).",
          bypassNeutral() },
    };
    return presets;
}

int findMasterPresetIndex (const char* id)
{
    if (id == nullptr)
        return -1;
    const auto& all = allMasterPresets();
    for (int i = 0; i < (int) all.size(); ++i)
        if (std::strcmp (all[(size_t) i].id, id) == 0)
            return i;
    return -1;
}

const MasterPreset* masterPresetAt (int index)
{
    const auto& all = allMasterPresets();
    if (index < 0 || index >= (int) all.size())
        return nullptr;
    return &all[(size_t) index];
}

} // namespace k3ch_master
