#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include <Unreal/CoreUObject/UObject/Class.hpp>
#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Structs/FWindroseLocalizedTextData.h"
#include "SDK/Classes/KismetInternationalizationLibrary.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "SDK/Classes/Custom/UDataTableStore.h"
#include "Loader/WindroseLanguageModLoader.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Helpers/String.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Windrose {
	WindroseLanguageModLoader::WindroseLanguageModLoader() : WindroseModLoaderBase("translations") {}

	WindroseLanguageModLoader::~WindroseLanguageModLoader() {}

    void WindroseLanguageModLoader::OnLoad(const fs::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase != EEngineLifecyclePhase::GameInstanceInit)
        {
            return;
        }

        auto globalLanguageFolder = loaderPath / "global";
        if (fs::exists(globalLanguageFolder))
        {
            PS::JsonHelpers::ParseJsonFilesInPath(globalLanguageFolder, [&](nlohmann::json data) {
                LoadTranslations(data);
            });
        }

        auto translationLanguageFolder = loaderPath / GetCurrentLanguage();
        if (fs::exists(translationLanguageFolder))
        {
            PS::JsonHelpers::ParseJsonFilesInPath(translationLanguageFolder, [&](nlohmann::json data) {
                LoadTranslations(data);
            });
        }
    }

    void WindroseLanguageModLoader::OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(modFilePath, [&](nlohmann::json data) {
            LoadTranslations(data);
        });
    }

    bool WindroseLanguageModLoader::CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::GameInstanceInit)
        {
            return true;
        }

        return false;
    }

    bool WindroseLanguageModLoader::OnInitialize()
    {
        try
        {
            auto config = PS::PSConfig::Get();
            auto languageOverride = config->GetLanguageOverride();

            if (languageOverride == "")
            {
                PS::Log<LogLevel::Verbose>(STR("Fetching current language from Kismet Internationalization Library...\n"));
                auto language = Windrose::UKismetInternationalizationLibrary::GetCurrentLanguage();
                m_currentLanguage = RC::to_string(*language);
                PS::Log<RC::LogLevel::Normal>(STR("Language override not set, using system language ({}).\n"), *language);
            }
            else
            {
                m_currentLanguage = languageOverride;
                PS::Log<RC::LogLevel::Normal>(STR("Language override set to {}.\n"), RC::to_generic_string(languageOverride));
            }
        }
        catch (const std::exception& e)
        {
            PS::Log<LogLevel::Error>(STR("Unable to initialize {}, {}\n"), GetDisplayName(), RC::to_generic_string(e.what()));
            return false;
        }

        return true;
    }

    void WindroseLanguageModLoader::LoadTranslations(const nlohmann::json& data)
    {
        for (auto& [TableName, TableData] : data.items())
        {
            for (auto& [RowId, RowValue] : TableData.items())
            {
                auto Table = TryGetDatatableByName(TableName);
                if (Table)
                {
                    auto RowStruct = Table->GetRowStruct();
                    if (!RowStruct.Get())
                    {
                        throw std::runtime_error("RowStruct was invalid");
                    }

                    static auto NAME_WindroseLocalizedTextData = FName(TEXT("WindroseLocalizedTextData"), FNAME_Add);
                    if (RowStruct.Get()->GetNamePrivate() != NAME_WindroseLocalizedTextData)
                    {
                        throw std::runtime_error("Row provided isn't equivalent to WindroseLocalizedTextData");
                    }

                    if (!RowValue.is_string())
                    {
                        throw std::runtime_error("String value must be provided for a translation");
                    }

                    auto RowName = FName(RC::to_generic_string(RowId), FNAME_Add);
                    auto Row = std::bit_cast<FWindroseLocalizedTextData*>(Table->FindRowUnchecked(RowName));
                    if (Row)
                    {
                        Row->TextData = FText(RC::to_generic_string(RowValue.get<std::string>()));
                        PS::Log<LogLevel::Normal>(STR("Localization row '{}' has been updated in {}\n"), RowName.ToString(), Table->GetName());
                    }
                    else
                    {
                        FWindroseLocalizedTextData NewRow{};
                        NewRow.TextData = FText(RC::to_generic_string(RowValue.get<std::string>()));
                        Table->AddRow(RowName, NewRow);
                        PS::Log<LogLevel::Normal>(STR("Localization row '{}' has been added to {}\n"), RowName.ToString(), Table->GetName());
                    }
                }
            }
        }
    }

	const std::string& WindroseLanguageModLoader::GetCurrentLanguage()
	{
		return m_currentLanguage;
	}
}