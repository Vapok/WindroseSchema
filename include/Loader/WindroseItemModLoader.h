#pragma once

#include "Loader/WindroseModLoaderBase.h"
#include "SDK/Classes/WindroseStaticItemDataAsset.h"
#include "SDK/Classes/WindroseDynamicItemDataBase.h"
#include "SDK/Structs/FWindroseItemId.h"
#include "safetyhook.hpp"

namespace RC::Unreal {
	class UDataTable;
}

namespace Windrose {
    class UWindroseStaticItemDataTable;

	class WindroseItemModLoader : public WindroseModLoaderBase {
	public:
		WindroseItemModLoader();

		~WindroseItemModLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual void OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
	private:
        void LoadItems(const nlohmann::json& data);

		void Add(const RC::Unreal::FName& itemId, const nlohmann::json& data);

		void Edit(const RC::Unreal::FName& itemId, UWindroseStaticItemDataBase* item, const nlohmann::json& data);

		void AddRecipe(const RC::Unreal::FName& itemId, const nlohmann::json& recipe);

		void EditRecipe(const RC::Unreal::FName& itemId, const nlohmann::json& recipe);

		void AddTranslations(const RC::Unreal::FName& itemId, const nlohmann::json& data);

		void EditTranslations(const RC::Unreal::FName& itemId, const nlohmann::json& data);

        // Handles DT_ItemDataTable stuff
        void AddItemData(const RC::Unreal::FName& itemId, const nlohmann::json& data);

        void SetupHooks();

		UWindroseStaticItemDataAsset* m_itemDataAsset{};
		RC::Unreal::UDataTable* m_itemDataTable{};
		RC::Unreal::UDataTable* m_itemRecipeTable{};
		RC::Unreal::UDataTable* m_nameTranslationTable{};
		RC::Unreal::UDataTable* m_descriptionTranslationTable{};
    private:
        static inline void* ApplyItemSaveDataAddress = nullptr;
        static inline void* ApplyDynamicItemSaveDataAddress = nullptr;
        static inline SafetyHookInline UpdateItem_ServerInternalHook;
        static inline SafetyHookInline DynamicItemHook;

        static bool IsValidItem(RC::Unreal::UObject* worldContextObject, const RC::Unreal::FName& staticId);
        static void UpdateItem_Detour(RC::Unreal::UObject* self, FWindroseItemId* itemId, int amount, bool param4, bool param5);
        static UWindroseDynamicItemDataBase* CreateDynamicItemDatabase_Detour(RC::Unreal::UObject* self, FWindroseDynamicItemId* dynamicItemId, RC::Unreal::FName staticId, void* itemCreateParam);
	};
}