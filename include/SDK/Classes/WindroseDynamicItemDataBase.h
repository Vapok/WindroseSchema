#pragma once

#include "Unreal/CoreUObject/UObject/UnrealType.hpp"

namespace Windrose {
    class UWindroseDynamicItemDataBase : public RC::Unreal::UObject {
    public:
        bool& GetIgnoreOnSave();
    };
}