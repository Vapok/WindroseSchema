#pragma once

#include "SDK/Structs/Reflected/BaseReflectedStruct.h"
#include "SDK/Structs/WindroseListInfo.h"

namespace Windrose {
    struct FWindroseSpawnerGroupInfo : UECustom::BaseReflectedStruct {
    public:
        FWindroseSpawnerGroupInfo();
        FWindroseSpawnerGroupInfo(void* Data);

        void SetWeight(int value);
        void SetOnlyTime(RC::Unreal::uint8 value);
        void SetOnlyWeather(RC::Unreal::uint8 value);
        void AddWindrose(const WindroseListInfo& value);

        virtual RC::Unreal::UScriptStruct* StaticStruct() final;
    };
}