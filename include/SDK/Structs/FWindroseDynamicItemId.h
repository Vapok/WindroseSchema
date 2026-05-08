#pragma once

namespace Windrose {
    struct FWindroseDynamicItemId {
        RC::Unreal::FGuid CreatedWorldId{};
        RC::Unreal::FGuid LocalIdInCreatedWorld{};
    };
}