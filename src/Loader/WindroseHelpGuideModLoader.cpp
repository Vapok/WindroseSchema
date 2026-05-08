#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/Engine/UDataTable.hpp"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "SDK/Structs/Custom/FManagedStruct.h"
#include "SDK/Classes/WindroseNoteDataAsset.h"
#include "SDK/Classes/WindroseNoteData.h"
#include "SDK/Helper/PropertyHelper.h"
#include "Helpers/String.hpp"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Loader/WindroseHelpGuideModLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
	WindroseHelpGuideModLoader::WindroseHelpGuideModLoader() : WindroseModLoaderBase("helpguide") {
        SetDisplayName(TEXT("Help Guide Loader"));
    }

	WindroseHelpGuideModLoader::~WindroseHelpGuideModLoader() {}

    void WindroseHelpGuideModLoader::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase != EEngineLifecyclePhase::GameInstanceInit)
        {
            return;
        }

        PS::JsonHelpers::ParseJsonFilesInPath(loaderPath, [&](const nlohmann::json& data) {
            LoadHelpGuides(data);
        });
    }

    void WindroseHelpGuideModLoader::OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(modFilePath, [&](const nlohmann::json& data) {
            LoadHelpGuides(data);
        });
    }

    bool WindroseHelpGuideModLoader::CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::GameInstanceInit)
        {
            return true;
        }

        return false;
    }

    bool WindroseHelpGuideModLoader::OnInitialize()
    {
        try
        {
            m_helpGuideDataAsset = UObjectGlobals::StaticFindObject<UWindroseNoteDataAsset*>(nullptr, nullptr,
                STR("/Game/Windrose/DataAsset/HelpGuide/DA_HelpGuideDataAsset.DA_HelpGuideDataAsset"));

            m_helpGuideDescTextTable = GetDatatableByName("DT_HelpGuideDescText");
        }
        catch (const std::exception& e)
        {
            PS::Log<LogLevel::Error>(STR("Unable to initialize {}, {}\n"), GetDisplayName(), RC::to_generic_string(e.what()));
            return false;
        }

        return true;
    }

    void WindroseHelpGuideModLoader::LoadHelpGuides(const nlohmann::json& data)
    {
        for (auto& [Key, Value] : data.items())
        {
            auto NoteId = FName(RC::to_generic_string(Key), FNAME_Add);

            if (Value.is_null())
            {
                // Delete from all sources
                auto Row = m_helpGuideDataAsset->NoteDataMap.Find(NoteId);
                if (Row)
                {
                    m_helpGuideDataAsset->NoteDataMap.Remove(NoteId);
                }

                DeleteRelatedData(NoteId);
                PS::Log<RC::LogLevel::Normal>(STR("Deleted Help Guide '{}'\n"), NoteId.ToString());
            }
            else
            {
                auto Row = m_helpGuideDataAsset->NoteDataMap.Find(NoteId);
                if (Row)
                {
                    Edit(NoteId, *Row, Value);
                    PS::Log<RC::LogLevel::Normal>(STR("Modified Help Guide '{}'\n"), NoteId.ToString());
                }
                else
                {
                    Add(NoteId, Value);
                    PS::Log<RC::LogLevel::Normal>(STR("Added Help Guide '{}'\n"), NoteId.ToString());
                }

                // Always update the DataTables
                AddOrEditDescText(NoteId, Value);
            }
        }
    }

	void WindroseHelpGuideModLoader::Add(const RC::Unreal::FName& NoteId, const nlohmann::json& Data)
	{
		if (NoteId == NAME_None)
		{
			throw std::runtime_error("ID was set to None");
		}

		UClass* WindroseNoteDataClass = UWindroseNoteData::StaticClass();
		if (!WindroseNoteDataClass)
        {
            throw std::runtime_error("Failed to find WindroseNoteData class. The class may have been renamed or removed.");
        }

        if (!WindroseNoteDataClass->GetPropertyByNameInChain(STR("TextId_Description")))
        {
            throw std::runtime_error("Property 'TextId_Description' has changed in DA_HelpGuideDataAsset. Update to Windrose Schema is needed.");
        }

        if (!WindroseNoteDataClass->GetPropertyByNameInChain(STR("Texture")))
		{
			throw std::runtime_error("Property 'Texture' has changed in DA_HelpGuideDataAsset. Update to Windrose Schema is needed.");
		}

		FStaticConstructObjectParameters ConstructParams(WindroseNoteDataClass, m_helpGuideDataAsset);
		ConstructParams.Name = NAME_None;

		auto NoteData = UObjectGlobals::StaticConstructObject<UWindroseNoteData*>(ConstructParams);

		// Set the TextId_Description to the NoteId by default
		auto TextIdProperty = NoteData->GetValuePtrByPropertyNameInChain<FName>(STR("TextId_Description"));
		if (TextIdProperty)
		{
			*TextIdProperty = NoteId;
		}

        if (Data.contains("Texture"))
        {
            auto TextureProperty = NoteData->GetPropertyByNameInChain(STR("Texture"));
            PropertyHelper::CopyJsonValueToContainer(reinterpret_cast<uint8_t*>(NoteData), TextureProperty, Data.at("Texture"));
        }

		m_helpGuideDataAsset->NoteDataMap.Add(NoteId, NoteData);
	}

	void WindroseHelpGuideModLoader::Edit(const RC::Unreal::FName& NoteId, UWindroseNoteData* NoteData, const nlohmann::json& Data)
	{
        for (FProperty* Property : TFieldRange<FProperty>(NoteData->GetClassPrivate(), EFieldIterationFlags::IncludeSuper))
        {
            auto PropertyName = RC::to_string(Property->GetName());
            if (Data.contains(PropertyName))
            {
                PropertyHelper::CopyJsonValueToContainer(reinterpret_cast<uint8_t*>(NoteData), Property, Data.at(PropertyName));
            }
        }
	}

	void WindroseHelpGuideModLoader::AddOrEditDescText(const RC::Unreal::FName& NoteId, const nlohmann::json& Data)
	{
		if (!m_helpGuideDescTextTable) return;

        std::string Title = "";
        std::string Description = "";

        if (Data.contains("Title") && Data.at("Title").is_string())
        {
            Title = Data.at("Title").get<std::string>();
        }

        if (Data.contains("Description") && Data.at("Description").is_string())
        {
            Description = Data.at("Description").get<std::string>();
        }

        if (Title == "")
        {
            Title = RC::to_string(NoteId.ToString());
        }

        if (!Title.ends_with("\r\n\r\n"))
        {
            Title = std::format("{}\r\n\r\n", Title);
        }

		auto TableRow = m_helpGuideDescTextTable->FindRowUnchecked(NoteId);
		auto TableRowStruct = m_helpGuideDescTextTable->GetRowStruct().Get();

		if (TableRow)
		{
            auto TextDataProperty = TableRowStruct->GetPropertyByNameInChain(STR("TextData"));
            if (TextDataProperty)
            {
                auto TextData = TextDataProperty->ContainerPtrToValuePtr<FText>(TableRow);
                auto FinalDescription = std::format("{}{}", Title, Description);
                auto FinalDescriptionWide = RC::to_generic_string(FinalDescription);
                *TextData = FText(FinalDescriptionWide.c_str());
            }
		}
		else
		{
            auto RowData = FManagedStruct(TableRowStruct);

            auto TextDataProperty = TableRowStruct->GetPropertyByNameInChain(STR("TextData"));
            if (TextDataProperty)
            {
                auto TextData = TextDataProperty->ContainerPtrToValuePtr<FText>(RowData.GetData());
                auto FinalDescription = std::format("{}{}", Title, Description);
                auto FinalDescriptionWide = RC::to_generic_string(FinalDescription);
                *TextData = FText(FinalDescriptionWide.c_str());
            }

            m_helpGuideDescTextTable->AddRow(NoteId, *reinterpret_cast<RC::Unreal::FTableRowBase*>(RowData.GetData()));
		}
	}

	void WindroseHelpGuideModLoader::DeleteRelatedData(const RC::Unreal::FName& NoteId)
	{
		if (m_helpGuideDescTextTable)
		{
			m_helpGuideDescTextTable->RemoveRow(NoteId);
		}
	}
}
