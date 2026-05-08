#include "SDK/Structs/Reflected/WindroseSpawnerGroupInfo.h"
#include "SDK/Structs/Reflected/WindroseSpawnerOneTribeInfo.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Unreal/Property/FEnumProperty.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    FWindroseSpawnerGroupInfo::FWindroseSpawnerGroupInfo() : BaseReflectedStruct(StaticStruct()) {}
    FWindroseSpawnerGroupInfo::FWindroseSpawnerGroupInfo(void* data) : BaseReflectedStruct(StaticStruct(), data) {};

    void FWindroseSpawnerGroupInfo::SetWeight(int value)
    {
        auto property = GetPropertyChecked<FNumericProperty>(STR("Weight"));
        SetPropertyValue<int>(property, value);
    }

    void FWindroseSpawnerGroupInfo::SetOnlyTime(RC::Unreal::uint8 value)
    {
        auto property = GetPropertyChecked<FEnumProperty>(STR("OnlyTime"));
        SetPropertyValue<RC::Unreal::uint8>(property, value);
    }

    void FWindroseSpawnerGroupInfo::SetOnlyWeather(RC::Unreal::uint8 value)
    {
        auto property = GetPropertyChecked<FEnumProperty>(STR("OnlyWeather"));
        SetPropertyValue<RC::Unreal::uint8>(property, value);
    }

    void FWindroseSpawnerGroupInfo::AddWindrose(const WindroseListInfo& value)
    {
        auto property = GetPropertyChecked<FArrayProperty>(STR("WindroseList"));
        auto array = GetArrayPropertyValue(property);

        UECustom::FManagedValue valuePtr;
        array->InitializeValue(valuePtr);

        FWindroseSpawnerOneTribeInfo tribeInfo(valuePtr.GetData());
        tribeInfo.SetWindroseId(value.WindroseId);
        tribeInfo.SetNPCID(value.NPCID);
        tribeInfo.SetLevel(value.Level);
        tribeInfo.SetLevelMax(value.LevelMax);
        tribeInfo.SetNum(value.Num);
        tribeInfo.SetNumMax(value.NumMax);

        array->Add(valuePtr);
    }

    RC::Unreal::UScriptStruct* FWindroseSpawnerGroupInfo::StaticStruct()
    {
        static auto Struct = UECustom::UObjectGlobals::StaticFindObject<UScriptStruct*>(nullptr, nullptr, STR("/Script/Windrose.WindroseSpawnerGroupInfo"));
        return Struct;
    }
}