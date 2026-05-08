#pragma once

#include "Unreal/UObject.hpp"

namespace Windrose {
    class UWindroseStaticItemDataAsset;

    class UWindroseStaticItemDataTable : public RC::Unreal::UObject {
    public:
        UWindroseStaticItemDataAsset* GetDataAsset();
    };
}