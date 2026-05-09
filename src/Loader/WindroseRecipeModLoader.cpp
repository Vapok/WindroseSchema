#include <fstream>
#include <Unreal/CoreUObject/UObject/UnrealType.hpp>
#include "Helpers/Casting.hpp"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "SDK/Classes/TSoftObjectPtr.h"
#include "SDK/Structs/FSoftObjectPath.h"
#include "SDK/Helper/PropertyHelper.h"
#include "UE4SSProgram.hpp"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "Loader/WindroseRecipeModLoader.h"
#include "nlohmann/json.hpp"

using namespace RC;
using namespace RC::Unreal;
using json = nlohmann::json;

namespace Windrose {

    static constexpr const TCHAR* RecipeDataClassPath = TEXT("/Script/R5BusinessRules.R5BLRecipeData");

    struct RecipeEntry {
        std::string asset;
        std::string tag;
        std::vector<std::pair<std::string, int>> cost;
        std::vector<std::pair<std::string, int>> result;
    };

    static RC::StringType FormatItems(const std::vector<std::pair<std::string, int>>& items)
    {
        if (items.empty()) return TEXT("<none>");
        RC::StringType out;
        for (auto& [path, count] : items)
        {
            if (!out.empty()) out += TEXT(", ");
            out += std::format(TEXT("{}x {}"), count, RC::to_generic_string(path));
        }
        return out;
    }

    static std::string CsvEscape(const std::string& s)
    {
        std::string out = "\"";
        for (char c : s) { if (c == '"') out += '"'; out += c; }
        out += '"';
        return out;
    }

    WindroseRecipeModLoader::WindroseRecipeModLoader() : WindroseModLoaderBase("recipes")
    {
        SetDisplayName(TEXT("Recipe Mod Loader"));
    }

    bool WindroseRecipeModLoader::CanInitialize(const EEngineLifecyclePhase& phase)
    {
        return phase == EEngineLifecyclePhase::GameInstanceInit;
    }

    bool WindroseRecipeModLoader::OnInitialize()
    {
        int found = DumpLoadedRecipes();
        if (found == 0)
        {
            PS::Log<LogLevel::Warning>(TEXT("[RecipeModLoader] No R5BLRecipeData found at GameInstanceInit. "
                "Recipes may load later — consider hooking a later phase.\n"));
        }
        return true;
    }

    void WindroseRecipeModLoader::OnLoad(const std::filesystem::path& loaderPath,
                                         const RC::StringType& modName,
                                         const EEngineLifecyclePhase& phase)
    {
        // Reserved for future JSON mod loading.
    }

    void WindroseRecipeModLoader::OnAutoReload(const std::filesystem::path::string_type& modName,
                                               const std::filesystem::path& modFilePath)
    {
        // Reserved.
    }

    // ---------------------------------------------------------------------------

    int WindroseRecipeModLoader::DumpLoadedRecipes()
    {
        auto recipeClass = UECustom::UObjectGlobals::StaticFindObject<UClass*>(
            nullptr, nullptr, RecipeDataClassPath, false);

        if (!recipeClass)
        {
            PS::Log<LogLevel::Error>(TEXT("[RecipeModLoader] Could not find UClass for {}. "
                "Is R5BusinessRules loaded?\n"), RecipeDataClassPath);
            return 0;
        }

        TArray<UObject*> recipes;
        UECustom::UObjectGlobals::GetObjectsOfClass(recipeClass, recipes, true,
            RF_NoFlags, EInternalObjectFlags::None);

        int count = recipes.Num();
        PS::Log<LogLevel::Normal>(TEXT("[RecipeModLoader] Found {} R5BLRecipeData objects.\n"), count);

        std::vector<RecipeEntry> entries;
        entries.reserve(static_cast<size_t>(count));

        for (auto* obj : recipes)
        {
            if (!obj) continue;

            auto assetName = obj->GetNamePrivate().ToString();
            if (assetName.starts_with(TEXT("Default__"))) continue;

            RecipeEntry e;
            e.asset  = RC::to_string(assetName);
            e.tag    = RC::to_string(ReadRecipeTag(obj));
            e.cost   = ReadItemArray(obj, TEXT("RecipeCost"));
            e.result = ReadItemArray(obj, TEXT("RecipeResult"));

            PS::Log<LogLevel::Normal>(
                TEXT("[RecipeModLoader]   [{}]\n")
                TEXT("                    Tag:    {}\n")
                TEXT("                    Cost:   {}\n")
                TEXT("                    Result: {}\n"),
                RC::to_generic_string(e.asset),
                RC::to_generic_string(e.tag),
                FormatItems(e.cost),
                FormatItems(e.result));

            entries.push_back(std::move(e));
        }

        PS::Log<LogLevel::Normal>(TEXT("[RecipeModLoader] Dumped {} recipes.\n"),
            static_cast<int>(entries.size()));

        if (!PS::PSConfig::Get()->IsRecipeDumpEnabled())
        {
            PS::Log<LogLevel::Normal>(TEXT("[RecipeModLoader] Recipe dump files disabled in config.\n"));
            return static_cast<int>(entries.size());
        }

        auto baseDir = std::filesystem::path(
            UE4SSProgram::get_program().get_working_directory()) / "Mods" / "WindroseSchema";

        // CSV
        {
            auto csvPath = baseDir / "recipe_dump.csv";
            std::ofstream csv(csvPath);
            csv << "Asset,RecipeTag,CostItems,ResultItems\n";
            for (auto& e : entries)
            {
                auto costStr   = RC::to_string(FormatItems(e.cost));
                auto resultStr = RC::to_string(FormatItems(e.result));
                csv << CsvEscape(e.asset)   << ","
                    << CsvEscape(e.tag)     << ","
                    << CsvEscape(costStr)   << ","
                    << CsvEscape(resultStr) << "\n";
            }
            PS::Log<LogLevel::Normal>(TEXT("[RecipeModLoader] Wrote {}\n"),
                RC::to_generic_string(csvPath.string()));
        }

        // JSON
        {
            auto jsonPath = baseDir / "recipe_dump.json";
            json jarray = json::array();
            for (auto& e : entries)
            {
                json jcost = json::array();
                for (auto& [path, cnt] : e.cost)
                    jcost.push_back({ {"item", path}, {"count", cnt} });

                json jresult = json::array();
                for (auto& [path, cnt] : e.result)
                    jresult.push_back({ {"item", path}, {"count", cnt} });

                jarray.push_back({
                    {"asset",  e.asset},
                    {"tag",    e.tag},
                    {"cost",   jcost},
                    {"result", jresult}
                });
            }
            std::ofstream jfile(jsonPath);
            jfile << jarray.dump(2);
            PS::Log<LogLevel::Normal>(TEXT("[RecipeModLoader] Wrote {}\n"),
                RC::to_generic_string(jsonPath.string()));
        }

        return static_cast<int>(entries.size());
    }

    RC::StringType WindroseRecipeModLoader::ReadRecipeTag(UObject* obj)
    {
        if (!obj) return TEXT("<null>");

        auto* objClass = obj->GetClassPrivate();
        if (!objClass) return TEXT("<no class>");

        // RecipeTag is FGameplayTag — find it as an FStructProperty on R5BLRecipeData.
        auto* recipeTagProp = CastField<FStructProperty>(
            PropertyHelper::GetPropertyByName(objClass, TEXT("RecipeTag")));
        if (!recipeTagProp) return TEXT("<no RecipeTag prop>");

        // GetStruct() returns TObjectPtr<UScriptStruct> — use operator-> for member access
        // without needing an implicit raw-pointer conversion.
        auto gamplayTagStruct = recipeTagProp->GetStruct();
        if (!gamplayTagStruct) return TEXT("<no struct>");

        // Get a pointer to the FGameplayTag value stored inside the recipe object.
        auto* recipeTagValuePtr = recipeTagProp->ContainerPtrToValuePtr<void>(obj);

        // Walk the FGameplayTag struct's property chain looking for TagName (FName).
        // Mirrors the SetStructPropertyValueFromJsonValue pattern — avoids the raw
        // UScriptStruct* conversion issue with TObjectPtr.
        for (FField* field = gamplayTagStruct->GetChildProperties(); field; field = PropertyHelper::GetNextField(field))
        {
            auto* prop = static_cast<FProperty*>(field);
            if (PropertyHelper::GetPropertyNameAsUTF8String(prop) == "TagName")
            {
                auto* tagNamePtr = prop->ContainerPtrToValuePtr<FName>(recipeTagValuePtr);
                if (tagNamePtr) return tagNamePtr->ToString();
                break;
            }
        }

        return TEXT("<TagName not found>");
    }

    int WindroseRecipeModLoader::ReadResultCount(UObject* obj)
    {
        if (!obj) return -1;

        auto* objClass = obj->GetClassPrivate();
        if (!objClass) return -1;

        auto* resultProp = CastField<FArrayProperty>(
            PropertyHelper::GetPropertyByName(objClass, TEXT("RecipeResult")));
        if (!resultProp) return -1;

        auto* arrayPtr = resultProp->ContainerPtrToValuePtr<FScriptArray>(obj);
        if (!arrayPtr) return -1;

        return static_cast<int>(arrayPtr->Num());
    }

    std::vector<std::pair<std::string, int>>
    WindroseRecipeModLoader::ReadItemArray(UObject* obj, const RC::StringType& propName)
    {
        std::vector<std::pair<std::string, int>> out;
        if (!obj) return out;

        auto* objClass = obj->GetClassPrivate();
        if (!objClass) return out;

        // ── NO DIAGNOSTIC LOGGING BELOW — production path ──

        auto* arrayProp = CastField<FArrayProperty>(
            PropertyHelper::GetPropertyByName(objClass, propName));
        if (!arrayProp) return out;

        auto* arrayPtr = arrayProp->ContainerPtrToValuePtr<FScriptArray>(obj);
        if (!arrayPtr || arrayPtr->Num() == 0) return out;

        auto* innerProp = CastField<FStructProperty>(arrayProp->GetInner());
        if (!innerProp) return out;

        auto itemStruct = innerProp->GetStruct();
        if (!itemStruct) return out;

        // Look up "Item" and "Count" by name directly — no linked-list walk,
        // no dependency on PropertyHelper::GetNextField (whose FField::Next
        // offset 0x20 is wrong for this build; actual Next is at 0x18).
        auto* countProp = CastField<FIntProperty>(
            PropertyHelper::GetPropertyByName(itemStruct, TEXT("Count")));
        auto* softProp  = CastField<FSoftObjectProperty>(
            PropertyHelper::GetPropertyByName(itemStruct, TEXT("Item")));

        if (!countProp && !softProp) return out;

        int32 elemSize = innerProp->GetElementSize();
        if (elemSize == 0) return out;

        uint8* dataBase = static_cast<uint8*>(arrayPtr->GetData());
        if (!dataBase) return out;

        for (int32 i = 0; i < arrayPtr->Num(); ++i)
        {
            uint8* elemPtr = dataBase + i * elemSize;
            int count = 0;
            std::string itemPath;

            if (countProp)
            {
                auto* countPtr = countProp->ContainerPtrToValuePtr<int32>(elemPtr);
                if (countPtr) count = *countPtr;
            }

            if (softProp)
            {
                // CppSDK-verified TSoftObjectPtr<T> layout (total 0x28 = 40 bytes):
                //   +0x00: FWeakObjectPtr (ObjectIndex int32 + ObjectSerialNumber int32)
                //   +0x08: FSoftObjectPath::AssetPath::PackageName FName
                //   +0x10: FSoftObjectPath::AssetPath::AssetName   FName
                //   +0x18: FSoftObjectPath::SubPathString FUtf8String (not read)
                const uint8* base = static_cast<const uint8*>(
                    softProp->ContainerPtrToValuePtr<void>(elemPtr));
                if (base)
                {
                    FName pkgName, astName;
                    memcpy(&pkgName, base + 0x08, sizeof(FName));
                    memcpy(&astName, base + 0x10, sizeof(FName));
                    if (astName != NAME_None)
                    {
                        itemPath = RC::to_string(std::format(TEXT("{}.{}"),
                            pkgName.ToString(), astName.ToString()));
                    }
                }
            }

            out.emplace_back(itemPath.empty() ? "<unknown>" : itemPath, count);
        }

        return out;
    }

}
