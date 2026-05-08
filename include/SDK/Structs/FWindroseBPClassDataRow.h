#pragma once

#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Classes/TSoftClassPtr.h"
#include "SDK/Classes/Texture2D.h"

namespace Windrose {
    struct FWindroseBPClassDataRow : public RC::Unreal::FTableRowBase
    {
        FWindroseBPClassDataRow(const RC::StringType& Path) : BPClass(UECustom::TSoftClassPtr<RC::Unreal::UClass>(UECustom::FSoftObjectPath(Path)))
        {
        }
        UECustom::TSoftClassPtr<RC::Unreal::UClass> BPClass;
    };
}