#pragma once

#include "Unreal/FText.hpp"
#include "Unreal/Engine/UDataTable.hpp"

namespace Windrose {
    struct FWindroseLocalizedTextData final : public RC::Unreal::FTableRowBase
    {
    public:
        RC::Unreal::FText TextData;
    };
}