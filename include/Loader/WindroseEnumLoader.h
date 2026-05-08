#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/WindroseModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace UECustom {
    class UDataAsset;
}

namespace RC::Unreal {
    class UEnum;
}

namespace Windrose {
    class WindroseEnumLoader : public WindroseModLoaderBase {
    public:
        WindroseEnumLoader();
        ~WindroseEnumLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        
        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        std::unordered_map<RC::StringType, RC::Unreal::UEnum*> m_enumNameToObjectMap;

        void LoadEnums(const nlohmann::json& data);

        RC::Unreal::UEnum* GetEnumByName(const RC::StringType& Name);
    };
}