#pragma once

#include <vector>
#include "SDK/Structs/WindroseListInfo.h"
#include "nlohmann/json_fwd.hpp"

namespace PS {
    struct WindroseSpawnGroupListInfo {
        int Weight = 10;
        RC::Unreal::uint8 OnlyTime = 0;
        RC::Unreal::uint8 OnlyWeather = 0;
        std::vector<Windrose::WindroseListInfo> WindroseList;

        void AddWindroseListInfo(const nlohmann::json& value);
    };
}