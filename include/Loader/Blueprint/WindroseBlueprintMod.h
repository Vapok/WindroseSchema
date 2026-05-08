#pragma once

#include <string>
#include "Unreal/NameTypes.hpp"
#include "nlohmann/json.hpp"

namespace Windrose {
    class WindroseBlueprintMod {
    public:
        WindroseBlueprintMod(const RC::Unreal::FName& blueprintName, const nlohmann::json& data);

        virtual ~WindroseBlueprintMod() {};
    public:
        const RC::Unreal::FName& GetBlueprintName() const;

        const nlohmann::json& GetData() const;
    private:
        RC::Unreal::FName m_name;
        nlohmann::json m_data;
    };
}