#include "Unreal/UObjectGlobals.hpp"
#include <Unreal/CoreUObject/UObject/UnrealType.hpp>
#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Classes/KismetInternationalizationLibrary.h"
#include "SDK/Structs/FWindroseCharacterIconDataRow.h"
#include "SDK/Structs/FWindroseBPClassDataRow.h"
#include "SDK/Helper/PropertyHelper.h"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Helpers/String.hpp"
#include "Loader/WindroseMonsterModLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
	WindroseMonsterModLoader::WindroseMonsterModLoader() : WindroseModLoaderBase("windrose") {
        SetDisplayName(TEXT("Windrose Loader"));
    }

	WindroseMonsterModLoader::~WindroseMonsterModLoader() {}

	void WindroseMonsterModLoader::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
	{
        if (engineLifecyclePhase != EEngineLifecyclePhase::GameInstanceInit)
        {
            return;
        }

        PS::JsonHelpers::ParseJsonFilesInPath(loaderPath, [&](const nlohmann::json& data) {
            LoadWindroseMonsters(data);
        });
	}

    void WindroseMonsterModLoader::OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(modFilePath, [&](const nlohmann::json& data) {
            LoadWindroseMonsters(data);
        });
    }

    bool WindroseMonsterModLoader::CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::GameInstanceInit)
        {
            return true;
        }

        return false;
    }

    bool WindroseMonsterModLoader::OnInitialize()
    {
        try
        {
            m_monsterDataTable = GetDatatableByName("DT_WindroseMonsterParameter");
            m_iconDataTable = GetDatatableByName("DT_WindroseCharacterIconDataTable");
            m_windroseBpClassTable = GetDatatableByName("DT_WindroseBPClass");
            m_wazaMasterLevelTable = GetDatatableByName("DT_WazaMasterLevel");
            m_windroseDropItemTable = GetDatatableByName("DT_WindroseDropItem");
            m_windroseNameTable = GetDatatableByName("DT_WindroseNameText");
            m_windroseShortDescTable = GetDatatableByName("DT_WindroseShortDescriptionText");
            m_windroseLongDescTable = GetDatatableByName("DT_WindroseLongDescriptionText");
        }
        catch (const std::exception& e)
        {
            PS::Log<LogLevel::Error>(STR("Unable to initialize {}, {}\n"), GetDisplayName(), RC::to_generic_string(e.what()));
            return false;
        }

        return true;
    }

    void WindroseMonsterModLoader::LoadWindroseMonsters(const nlohmann::json& data)
    {
        for (auto& [character_id, properties] : data.items())
        {
            auto CharacterId = FName(RC::to_generic_string(character_id), FNAME_Add);
            auto TableRow = m_monsterDataTable->FindRowUnchecked(CharacterId);
            if (TableRow)
            {
                Edit(TableRow, CharacterId, properties);
            }
            else
            {
                Add(CharacterId, properties);
            }
        }
    }

	void WindroseMonsterModLoader::Add(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties)
	{
		auto MonsterRowStruct = m_monsterDataTable->GetRowStruct().Get();
		auto MonsterRowData = FMemory::Malloc(MonsterRowStruct->GetStructureSize());
		MonsterRowStruct->InitializeStruct(MonsterRowData);

		for (auto& [key, value] : properties.items())
		{
			auto KeyName = RC::to_generic_string(key);
			if (KeyName == STR("IconAssetPath"))
			{
				auto IconPath = RC::to_generic_string(value.get<std::string>());
				AddIcon(CharacterId, IconPath);
			}
			else if (KeyName == STR("BlueprintAssetPath"))
			{
				auto BlueprintPath = RC::to_generic_string(value.get<std::string>());
				AddBlueprint(CharacterId, BlueprintPath);
			}
			else if (KeyName == STR("AbilitiesByLevel"))
			{
				AddAbilities(CharacterId, value);
			}
			else if (KeyName == STR("Loot"))
			{
				AddLoot(CharacterId, value);
			}
			else
			{
				auto Property = MonsterRowStruct->GetPropertyByName(KeyName.c_str());
				if (Property)
				{
					PropertyHelper::CopyJsonValueToContainer(MonsterRowData, Property, value);
				}
			}
		}

        // IsWindrose defaults false; set true for rows owned by this loader (Windrose creature tables).
        auto IsWindroseProp = MonsterRowStruct->GetPropertyByName(STR("IsWindrose"));
        if (IsWindroseProp)
        {
            *IsWindroseProp->ContainerPtrToValuePtr<bool>(MonsterRowData) = true;
        }

        m_monsterDataTable->AddRow(CharacterId, *reinterpret_cast<RC::Unreal::FTableRowBase*>(MonsterRowData));

        AddTranslations(CharacterId, properties);

        PS::Log<RC::LogLevel::Normal>(STR("Added new creature '{}'\n"), CharacterId.ToString());
    }

    void WindroseMonsterModLoader::Edit(uint8_t* TableRow, const RC::Unreal::FName& CharacterId, const nlohmann::json& properties)
    {
        auto RowStruct = m_monsterDataTable->GetRowStruct().Get();
		for (auto& [key, value] : properties.items())
		{
			auto KeyName = RC::to_generic_string(key);
			if (KeyName == STR("IconAssetPath"))
			{
				auto IconTableRow = std::bit_cast<FWindroseCharacterIconDataRow*>(m_iconDataTable->FindRowUnchecked(CharacterId));
				if (IconTableRow)
				{
					auto IconPath = RC::to_generic_string(value.get<std::string>());
					IconTableRow->Icon = UECustom::TSoftObjectPtr<UECustom::UTexture2D>(UECustom::FSoftObjectPath(IconPath));
				}
			}
			else if (KeyName == STR("ActorClassPath"))
			{
				auto BlueprintTableRow = std::bit_cast<FWindroseBPClassDataRow*>(m_windroseBpClassTable->FindRowUnchecked(CharacterId));
				if (BlueprintTableRow)
				{
					auto BlueprintPath = RC::to_generic_string(value.get<std::string>());
					BlueprintTableRow->BPClass = UECustom::TSoftClassPtr<RC::Unreal::UClass>(UECustom::FSoftObjectPath(BlueprintPath));
				}
			}
            else if (KeyName == STR("Loot"))
            {
                AddLoot(CharacterId, value);
            }
			else
			{
				auto Property = RowStruct->GetPropertyByName(KeyName.c_str());
				if (Property)
				{
					PropertyHelper::CopyJsonValueToContainer(TableRow, Property, value);
				}
			}
		}

		EditTranslations(CharacterId, properties);
	}

	void WindroseMonsterModLoader::AddIcon(const RC::Unreal::FName& CharacterId, const RC::StringType& IconPath)
	{
		FWindroseCharacterIconDataRow IconDataRow{ IconPath };
		m_iconDataTable->AddRow(CharacterId, IconDataRow);
	}

	void WindroseMonsterModLoader::AddBlueprint(const RC::Unreal::FName& CharacterId, const RC::StringType& BlueprintPath)
	{
		FWindroseBPClassDataRow BlueprintDataRow{ BlueprintPath };
		m_windroseBpClassTable->AddRow(CharacterId, BlueprintDataRow);
	}

	void WindroseMonsterModLoader::AddAbilities(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties)
	{
		auto RowStruct = m_wazaMasterLevelTable->GetRowStruct().Get();

		for (auto& [key, value] : properties.items())
		{
			if (!value.contains("WazaID"))
			{
				PS::Log<RC::LogLevel::Error>(STR("WazaID was not specified in {}, skipping ability entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!value.contains("Level"))
			{
				PS::Log<RC::LogLevel::Error>(STR("Level was not specified in {}, skipping ability entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!value.at("WazaID").is_string())
			{
				PS::Log<RC::LogLevel::Error>(STR("WazaID in {} must be a string, skipping ability entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!value.at("Level").is_number_integer())
			{
				PS::Log<RC::LogLevel::Error>(STR("Level in {} must be an integer, skipping ability entry.\n"), CharacterId.ToString());
				continue;
			}

			auto Level = value.at("Level").get<int>();

			auto WazaMasterLevelData = FMemory::Malloc(RowStruct->GetStructureSize());
			RowStruct->InitializeStruct(WazaMasterLevelData);

			auto WindroseIDProperty = RowStruct->GetPropertyByName(STR("WindroseID"));
			if (WindroseIDProperty)
			{
				FMemory::Memcpy(WindroseIDProperty->ContainerPtrToValuePtr<void>(WazaMasterLevelData), &CharacterId, sizeof(FName));
			}

			auto WazaIDProperty = RowStruct->GetPropertyByName(STR("WazaID"));
			if (WazaIDProperty)
			{
				PropertyHelper::CopyJsonValueToContainer(WazaMasterLevelData, WazaIDProperty, value.at("WazaID"));
			}

			auto LevelProperty = RowStruct->GetPropertyByName(STR("Level"));
			if (LevelProperty)
			{
				FMemory::Memcpy(LevelProperty->ContainerPtrToValuePtr<void>(WazaMasterLevelData), &Level, sizeof(int));
			}

			auto NewRow = reinterpret_cast<RC::Unreal::FTableRowBase*>(WazaMasterLevelData);
			auto NewRowName = std::format(STR("{}{}"), CharacterId.ToString(), Level);

			m_wazaMasterLevelTable->AddRow(FName(NewRowName, FNAME_Add), *NewRow);
		}
	}

	void WindroseMonsterModLoader::AddLoot(const RC::Unreal::FName& CharacterId, const nlohmann::json& properties)
	{
		auto RowStruct = m_windroseDropItemTable->GetRowStruct().Get();

		auto WindroseDropItemData = FMemory::Malloc(RowStruct->GetStructureSize());
		RowStruct->InitializeStruct(WindroseDropItemData);

		auto CharacterIdProperty = RowStruct->GetPropertyByName(STR("CharacterId"));
		if (!CharacterIdProperty)
		{
			FMemory::Free(WindroseDropItemData);
			throw std::runtime_error("Property CharacterId doesn't exist in DT_WindroseDropItem, which means you should wait for an update as something in the table has changed.");
		}

		FMemory::Memcpy(CharacterIdProperty->ContainerPtrToValuePtr<void>(WindroseDropItemData), &CharacterId, sizeof(FName));

		auto Index = 1;
		auto loot_array = properties.get<std::vector<nlohmann::json>>();
		for (auto& loot : loot_array)
		{
			auto IndexString = std::to_wstring(Index);

			if (!loot.contains("ItemId"))
			{
				PS::Log<RC::LogLevel::Error>(STR("ItemId was not specified in {}, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.contains("DropChance"))
			{
				PS::Log<RC::LogLevel::Error>(STR("DropChance was not specified in {}, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.contains("Min"))
			{
				PS::Log<RC::LogLevel::Error>(STR("Min was not specified in {}, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.contains("Max"))
			{
				PS::Log<RC::LogLevel::Error>(STR("Max was not specified in {}, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.at("ItemId").is_string())
			{
				PS::Log<RC::LogLevel::Error>(STR("ItemId in {} must be a string, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.at("DropChance").is_number_float())
			{
				PS::Log<RC::LogLevel::Error>(STR("DropChance in {} must be a float, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.at("Min").is_number_integer())
			{
				PS::Log<RC::LogLevel::Error>(STR("Min in {} must be an integer, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			if (!loot.at("Max").is_number_integer())
			{
				PS::Log<RC::LogLevel::Error>(STR("Max in {} must be an integer, skipping loot entry.\n"), CharacterId.ToString());
				continue;
			}

			auto ItemIdWithSuffix = std::format(STR("ItemId{}"), IndexString);
			auto ItemIdProperty = RowStruct->GetPropertyByName(ItemIdWithSuffix.c_str());
			if (!ItemIdProperty)
			{
				throw std::runtime_error(std::format("Property 'ItemId{}' doesn't exist in DT_WindroseDropItem, Windrose Schema needs an update.", Index));
			}

			auto RateWithSuffix = std::format(STR("Rate{}"), IndexString);
			auto RateProperty = RowStruct->GetPropertyByName(RateWithSuffix.c_str());
			if (!RateProperty)
			{
				throw std::runtime_error(std::format("Property 'Rate{}' doesn't exist in DT_WindroseDropItem, Windrose Schema needs an update.", Index));
			}

			auto MaxWithSuffix = std::format(STR("Max{}"), IndexString);
			auto MaxProperty = RowStruct->GetPropertyByName(MaxWithSuffix.c_str());
			if (!MaxProperty)
			{
				throw std::runtime_error(std::format("Property 'Max{}' doesn't exist in DT_WindroseDropItem, Windrose Schema needs an update.", Index));
			}

			auto MinWithSuffix = std::format(STR("min{}"), IndexString);
			auto MinProperty = RowStruct->GetPropertyByName(MinWithSuffix.c_str());
			if (!MinProperty)
			{
				throw std::runtime_error(std::format("Property 'min{}' doesn't exist in DT_WindroseDropItem, Windrose Schema needs an update.", Index));
			}

			auto ItemId = loot.at("ItemId");
			auto DropChance = loot.at("DropChance");
			auto Min = loot.at("Min");
			auto Max = loot.at("Max");

			PropertyHelper::CopyJsonValueToContainer(WindroseDropItemData, ItemIdProperty, ItemId);
			PropertyHelper::CopyJsonValueToContainer(WindroseDropItemData, RateProperty, DropChance);
			PropertyHelper::CopyJsonValueToContainer(WindroseDropItemData, MinProperty, Min);
			PropertyHelper::CopyJsonValueToContainer(WindroseDropItemData, MaxProperty, Max);

			Index++;

			if (Index > 10)
			{
				break;
			}
		}

		auto RowName = std::format(STR("{}000"), CharacterId.ToString());
		m_windroseDropItemTable->AddRow(FName(RowName, FNAME_Add), *reinterpret_cast<RC::Unreal::FTableRowBase*>(WindroseDropItemData));
	}

	void WindroseMonsterModLoader::AddTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data)
	{
		if (Data.contains("Name"))
		{
			auto FixedCharacterId = std::format(STR("PAL_NAME_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseNameTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto TranslationRowData = FMemory::Malloc(TranslationRowStruct->GetStructureSize());
				TranslationRowStruct->InitializeStruct(TranslationRowData);

				try
				{
					PropertyHelper::CopyJsonValueToContainer(TranslationRowData, TextProperty, Data.at("Name"));
				}
				catch (const std::exception& e)
				{
					FMemory::Free(TranslationRowData);
					throw std::runtime_error(e.what());
				}

				m_windroseNameTable->AddRow(FName(FixedCharacterId, FNAME_Add), *reinterpret_cast<RC::Unreal::FTableRowBase*>(TranslationRowData));
			}
		}

		if (Data.contains("ShortDescription"))
		{
			auto FixedCharacterId = std::format(STR("PAL_SHORT_DESC_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseShortDescTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto TranslationRowData = FMemory::Malloc(TranslationRowStruct->GetStructureSize());
				TranslationRowStruct->InitializeStruct(TranslationRowData);

				try
				{
					PropertyHelper::CopyJsonValueToContainer(TranslationRowData, TextProperty, Data.at("ShortDescription"));
				}
				catch (const std::exception& e)
				{
					FMemory::Free(TranslationRowData);
					throw std::runtime_error(e.what());
				}

				m_windroseShortDescTable->AddRow(FName(FixedCharacterId, FNAME_Add), *reinterpret_cast<RC::Unreal::FTableRowBase*>(TranslationRowData));
			}
		}

		if (Data.contains("LongDescription"))
		{
			auto FixedCharacterId = std::format(STR("PAL_LONG_DESC_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseLongDescTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto TranslationRowData = FMemory::Malloc(TranslationRowStruct->GetStructureSize());
				TranslationRowStruct->InitializeStruct(TranslationRowData);

				try
				{
					PropertyHelper::CopyJsonValueToContainer(TranslationRowData, TextProperty, Data.at("LongDescription"));
				}
				catch (const std::exception& e)
				{
					FMemory::Free(TranslationRowData);
					throw std::runtime_error(e.what());
				}

				m_windroseLongDescTable->AddRow(FName(FixedCharacterId, FNAME_Add), *reinterpret_cast<RC::Unreal::FTableRowBase*>(TranslationRowData));
			}
		}
	}

	void WindroseMonsterModLoader::EditTranslations(const RC::Unreal::FName& CharacterId, const nlohmann::json& Data)
	{
		if (Data.contains("Name"))
		{
			auto FixedCharacterId = std::format(STR("PAL_NAME_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseNameTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto Row = m_windroseNameTable->FindRowUnchecked(FName(FixedCharacterId, FNAME_Add));
				if (Row)
				{
					PropertyHelper::CopyJsonValueToContainer(Row, TextProperty, Data.at("Name"));
				}
			}
		}

		if (Data.contains("ShortDescription"))
		{
			auto FixedCharacterId = std::format(STR("PAL_SHORT_DESC_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseShortDescTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto Row = m_windroseShortDescTable->FindRowUnchecked(FName(FixedCharacterId, FNAME_Add));
				if (Row)
				{
					PropertyHelper::CopyJsonValueToContainer(Row, TextProperty, Data.at("ShortDescription"));
				}
			}
		}

		if (Data.contains("LongDescription"))
		{
			auto FixedCharacterId = std::format(STR("PAL_LONG_DESC_{}"), CharacterId.ToString());
			auto TranslationRowStruct = m_windroseLongDescTable->GetRowStruct().Get();
			auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
			if (TextProperty)
			{
				auto Row = m_windroseLongDescTable->FindRowUnchecked(FName(FixedCharacterId, FNAME_Add));
				if (Row)
				{
					PropertyHelper::CopyJsonValueToContainer(Row, TextProperty, Data.at("LongDescription"));
				}
			}
		}
	}
}