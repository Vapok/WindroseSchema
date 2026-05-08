#pragma once

#include "SDK/Structs/FWindroseDataTableRowName_WindroseMonsterData.h"
#include "SDK/Structs/FWindroseDataTableRowName_WindroseHumanData.h"

namespace Windrose {
    struct FWindroseSpawnerOneTribeInfo
    {
        FWindroseDataTableRowName_WindroseMonsterData WindroseID;
        FWindroseDataTableRowName_WindroseHumanData NPCID;
        int Level = 1;
        int Level_Max = 1;
        int Num = 1;
        int Num_Max = 1;
    };
}