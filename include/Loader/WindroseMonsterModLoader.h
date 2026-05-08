#pragma once

#include "Loader/WindroseModLoaderBase.h"
#include "Loader/WindroseLanguageModLoader.h"
#include "nlohmann/json.hpp"

namespace RC::Unreal {
    class UDataTable;
}

namespace Windrose {
    class WindroseMonsterModLoader : public WindroseModLoaderBase {
    public:
        WindroseMonsterModLoader();

        ~WindroseMonsterModLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual void OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        void LoadWindroseMonsters(const nlohmann::json& data);

        void Add(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

        void Edit(uint8_t* TableRow, const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

        void AddIcon(const RC::Unreal::FName& CharacterId, const RC::StringType& IconPath);

        void AddBlueprint(const RC::Unreal::FName& CharacterId, const RC::StringType& BlueprintPath);

        void AddAbilities(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

        void AddLoot(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

        void AddTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data);

        void EditTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data);

        RC::Unreal::UDataTable* m_monsterDataTable;
        RC::Unreal::UDataTable* m_iconDataTable;
        RC::Unreal::UDataTable* m_windroseBpClassTable;
        RC::Unreal::UDataTable* m_wazaMasterLevelTable;
        RC::Unreal::UDataTable* m_windroseDropItemTable;
        RC::Unreal::UDataTable* m_windroseNameTable;
        RC::Unreal::UDataTable* m_windroseShortDescTable;
        RC::Unreal::UDataTable* m_windroseLongDescTable;
    };
}