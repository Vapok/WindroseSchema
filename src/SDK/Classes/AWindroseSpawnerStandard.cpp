#include "SDK/Classes/AWindroseSpawnerStandard.h"
#include "SDK/Helper/PropertyHelper.h"
#include "SDK/Structs/Custom/FScriptArrayHelper.h"
#include "SDK/Structs/Reflected/WindroseSpawnerGroupInfo.h"
#include "Unreal/Core/Containers/ScriptArray.hpp"
#include <Unreal/CoreUObject/UObject/UnrealType.hpp>

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    void AWindroseSpawnerStandard::AddSpawnerGroup(const WindroseSpawnerGroup& spawnerGroup) {
        auto spawnGroupListProperty = PropertyHelper::GetPropertyByName(this->GetClassPrivate(), TEXT("SpawnGroupList"));
        if (!spawnGroupListProperty)
        {
            throw std::runtime_error("Could not get SpawnGroupList in AWindroseSpawnerStandard.");
        }

        auto spawnGroupList = spawnGroupListProperty->ContainerPtrToValuePtr<void>(this);

        auto scriptArray = static_cast<FScriptArray*>(spawnGroupList);
        auto scriptArrayHelper = UECustom::FScriptArrayHelper(scriptArray, static_cast<FArrayProperty*>(spawnGroupListProperty));

        UECustom::FManagedValue valuePtr;
        scriptArrayHelper.InitializeValue(valuePtr);

        FWindroseSpawnerGroupInfo spawnerGroupInfo(valuePtr.GetData());
        spawnerGroupInfo.SetWeight(spawnerGroup.Weight);
        spawnerGroupInfo.SetOnlyTime(spawnerGroup.OnlyTime);
        spawnerGroupInfo.SetOnlyWeather(spawnerGroup.OnlyWeather);

        for (auto& listEntry : spawnerGroup.WindroseList)
        {
            spawnerGroupInfo.AddWindrose(listEntry);
        }

        scriptArrayHelper.Add(valuePtr);
    }

    void AWindroseSpawnerStandard::SetSpawnerName(const RC::Unreal::FName& spawnerName)
    {
        auto property = PropertyHelper::GetPropertyByName(this->GetClassPrivate(), TEXT("SpawnerName"));
        *property->ContainerPtrToValuePtr<RC::Unreal::FName>(this) = spawnerName;
    }

    void AWindroseSpawnerStandard::SetSpawnerType(const RC::Unreal::uint8& spawnerType)
    {
        auto property = PropertyHelper::GetPropertyByName(this->GetClassPrivate(), TEXT("SpawnerType"));
        *property->ContainerPtrToValuePtr<RC::Unreal::uint8>(this) = spawnerType;
    }
}