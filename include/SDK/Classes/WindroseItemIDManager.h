#pragma once

#include "Unreal/UObject.hpp"

namespace Windrose {
    class UWindroseStaticItemDataTable;

    class UWindroseItemIDManager : public RC::Unreal::UObject {
    public:
        UWindroseStaticItemDataTable* GetStaticItemDataTable();

        RC::Unreal::UObject* GetStaticItemData(const RC::Unreal::FName& StaticItemId);
    };
}