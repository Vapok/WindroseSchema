#pragma once

#include "Unreal/UObject.hpp"
#include "SDK/Classes/WindroseStaticItemDataBase.h"

namespace Windrose {
    class UWindroseStaticArmorItemData : public UWindroseStaticItemDataBase {
    public:
        static RC::Unreal::UClass* StaticClass();
    };
}