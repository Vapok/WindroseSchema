#include "SDK/Structs/Reflected/WindroseSpawnerOneTribeInfo.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include <Unreal/CoreUObject/UObject/UnrealType.hpp>

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    FWindroseSpawnerOneTribeInfo::FWindroseSpawnerOneTribeInfo() : BaseReflectedStruct(StaticStruct()) {}
    FWindroseSpawnerOneTribeInfo::FWindroseSpawnerOneTribeInfo(void* data) : BaseReflectedStruct(StaticStruct(), data) {};

    void FWindroseSpawnerOneTribeInfo::SetWindroseId(const RC::Unreal::FName& value)
    {
        auto property = GetProperty(STR("WindroseId"));
        SetPropertyValue<RC::Unreal::FName>(property, value);
    }

    void FWindroseSpawnerOneTribeInfo::SetNPCID(const RC::Unreal::FName& value)
    {
        auto property = GetProperty(STR("NPCID"));
        SetPropertyValue<RC::Unreal::FName>(property, value);
    }

    void FWindroseSpawnerOneTribeInfo::SetLevel(int value)
    {
        auto property = GetPropertyChecked<FNumericProperty>(STR("Level"));
        SetPropertyValue<int>(property, value);
    }

    void FWindroseSpawnerOneTribeInfo::SetLevelMax(int value)
    {
        auto property = GetPropertyChecked<FNumericProperty>(STR("Level_Max"));
        SetPropertyValue<int>(property, value);
    }

    void FWindroseSpawnerOneTribeInfo::SetNum(int value)
    {
        auto property = GetPropertyChecked<FNumericProperty>(STR("Num"));
        SetPropertyValue<int>(property, value);
    }

    void FWindroseSpawnerOneTribeInfo::SetNumMax(int value)
    {
        auto property = GetPropertyChecked<FNumericProperty>(STR("Num_Max"));
        SetPropertyValue<int>(property, value);
    }

    RC::Unreal::UScriptStruct* FWindroseSpawnerOneTribeInfo::StaticStruct()
    {
        static auto Struct = UECustom::UObjectGlobals::StaticFindObject<UScriptStruct*>(nullptr, nullptr, STR("/Script/Windrose.WindroseSpawnerOneTribeInfo"));
        return Struct;
    }
}