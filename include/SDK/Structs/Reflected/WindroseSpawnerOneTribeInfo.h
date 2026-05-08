#pragma once

#include "SDK/Structs/Reflected/BaseReflectedStruct.h"

namespace Windrose {
    struct FWindroseSpawnerOneTribeInfo : UECustom::BaseReflectedStruct {
    public:
        FWindroseSpawnerOneTribeInfo();
        FWindroseSpawnerOneTribeInfo(void* Data);

        void SetWindroseId(const RC::Unreal::FName& value);
        void SetNPCID(const RC::Unreal::FName& value);
        void SetLevel(int value);
        void SetLevelMax(int value);
        void SetNum(int value);
        void SetNumMax(int value);

        virtual RC::Unreal::UScriptStruct* StaticStruct() final;
    };
}