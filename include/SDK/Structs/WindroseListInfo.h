#pragma once

#include "Unreal/NameTypes.hpp"

namespace Windrose {
    // Do not pass to UE internals, see FWindroseSpawnerOneTribeInfo and FWindroseSpawnerGroupInfo instead in /Reflected
    struct WindroseListInfo {
        RC::Unreal::FName WindroseId = RC::Unreal::NAME_None;
        RC::Unreal::FName NPCID = RC::Unreal::NAME_None;
        int Level = 1;
        int LevelMax = 1;
        int Num = 1;
        int NumMax = 1;
    };
}