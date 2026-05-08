#pragma once

#include <set>
#include "Loader/WindroseModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace RC::Unreal {
    class UDataTable;
}

namespace Windrose {
	class WindroseHumanModLoader : public WindroseModLoaderBase {
	public:
		WindroseHumanModLoader();

		~WindroseHumanModLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        void LoadHumans(const nlohmann::json& data);

		void Add(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

        void Edit(uint8_t* TableRow, const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

		void AddBlueprint(const RC::Unreal::FName& CharacterId, const RC::StringType& BlueprintPath);

		void AddIcon(const RC::Unreal::FName& CharacterId, const RC::StringType& IconPath);

		void AddLoot(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

		void AddTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data);

        void EditTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data);

		void AddShop(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties);

		RC::Unreal::UDataTable* m_humanDataTable = nullptr;
		RC::Unreal::UDataTable* m_iconDataTable = nullptr;
		RC::Unreal::UDataTable* m_windroseBpClassTable = nullptr;
		RC::Unreal::UDataTable* m_dropItemTable = nullptr;
		RC::Unreal::UDataTable* m_npcNameTable = nullptr;
        RC::Unreal::UDataTable* m_windroseShortDescTable = nullptr;
        RC::Unreal::UDataTable* m_windroseLongDescTable = nullptr;
		RC::Unreal::UDataTable* m_npcTalkFlowTable = nullptr;
		RC::Unreal::UDataTable* m_itemShopLotteryDataTable = nullptr;
		RC::Unreal::UDataTable* m_itemShopCreateDataTable = nullptr;
		RC::Unreal::UDataTable* m_itemShopSettingDataTable = nullptr;
	};
}