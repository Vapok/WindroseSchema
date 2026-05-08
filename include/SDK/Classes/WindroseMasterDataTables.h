#pragma once

#include "Unreal/UObject.hpp"

namespace RC::Unreal {
    class UDataTable;
}

namespace Windrose {
    class UWindroseMasterDataTables : public RC::Unreal::UObject {
    public:
        RC::Unreal::UDataTable* GetItemRecipeDataTable();
    };
}