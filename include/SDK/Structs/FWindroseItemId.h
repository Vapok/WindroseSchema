#pragma once

#include "SDK/Structs/FWindroseDynamicItemId.h"

namespace Windrose {
    struct FWindroseItemId {
        RC::Unreal::FName StaticId;
        FWindroseDynamicItemId DynamicId;
    };
}