#pragma once

#include "Unreal/AActor.hpp"
#include "SDK/Structs/WindroseSpawnerGroup.h"

namespace Windrose {
    class AWindroseSpawnerStandard : public RC::Unreal::AActor {
    public:
        void AddSpawnerGroup(const WindroseSpawnerGroup& spawnerGroup);

        void SetSpawnerName(const RC::Unreal::FName& spawnerName);

        void SetSpawnerType(const RC::Unreal::uint8& spawnerType);
    };
}