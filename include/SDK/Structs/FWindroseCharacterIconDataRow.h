#pragma once

#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Classes/TSoftObjectPtr.h"
#include "SDK/Classes/Texture2D.h"

namespace Windrose {
    struct FWindroseCharacterIconDataRow : public RC::Unreal::FTableRowBase
    {
        FWindroseCharacterIconDataRow(const RC::StringType& Path) : Icon(UECustom::TSoftObjectPtr<UECustom::UTexture2D>(UECustom::FSoftObjectPath(Path)))
        {
        }

        UECustom::TSoftObjectPtr<UECustom::UTexture2D> Icon;
    };
}