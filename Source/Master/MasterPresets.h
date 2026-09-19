#pragma once

#include "MasterDsp.h"

#include <vector>

namespace k3ch_master
{

struct MasterPreset
{
    const char* id = "";
    const char* name = "";
    const char* notes = "";
    MasterSettings settings;
};

const std::vector<MasterPreset>& allMasterPresets();
int findMasterPresetIndex (const char* id);
const MasterPreset* masterPresetAt (int index);

} // namespace k3ch_master
