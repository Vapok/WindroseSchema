#include "Loader/Blueprint/WindroseBlueprintMod.h"
#include <Helpers/String.hpp>

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    WindroseBlueprintMod::WindroseBlueprintMod(const RC::Unreal::FName& blueprintName, const nlohmann::json& data) : m_name(blueprintName), m_data(data) {}

    const FName& WindroseBlueprintMod::GetBlueprintName() const
    {
        return m_name;
    }

    const nlohmann::json& WindroseBlueprintMod::GetData() const
    {
        return m_data;
    }
}