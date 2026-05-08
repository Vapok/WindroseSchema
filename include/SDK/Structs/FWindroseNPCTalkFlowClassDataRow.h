#pragma once

#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Classes/TSoftClassPtr.h"

namespace Windrose {
    struct FWindroseNPCTalkFlowClassDataRow : public RC::Unreal::FTableRowBase
    {
        FWindroseNPCTalkFlowClassDataRow(const RC::StringType& Path) : NPCTalkFlowClass(UECustom::TSoftClassPtr<RC::Unreal::UClass>(UECustom::FSoftObjectPath(Path)))
        {
        }
        UECustom::TSoftClassPtr<RC::Unreal::UClass> NPCTalkFlowClass;
    };
}