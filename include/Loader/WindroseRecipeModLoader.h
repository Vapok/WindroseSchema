#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Loader/WindroseModLoaderBase.h"

namespace Windrose {
    // Loads recipe mod files from mods/<ModName>/recipes/*.json[c].
    //
    // Phase: GameInstanceInit.
    //   - On initialize: scans all loaded R5BLRecipeData objects via GetObjectsOfClass
    //     and logs a diagnostic dump of each recipe tag + result count.
    //   - On load: (reserved — mod JSON loading not yet implemented).
    //
    // This loader is the foundation for recipe patching. The diagnostic dump
    // establishes timing (are recipes in memory at GameInstanceInit?) and gives
    // modders the RecipeTag strings they need to identify recipes without FModel.
    class WindroseRecipeModLoader : public WindroseModLoaderBase {
    public:
        WindroseRecipeModLoader();

    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath,
                            const RC::StringType& modName,
                            const EEngineLifecyclePhase& engineLifecyclePhase) override final;

        virtual void OnAutoReload(const std::filesystem::path::string_type& modName,
                                  const std::filesystem::path& modFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;

        virtual bool OnInitialize() override final;

    private:
        // Dumps every loaded R5BLRecipeData to the UE4SS log.
        // Returns the number of recipes found (0 = not yet loaded).
        int DumpLoadedRecipes();

        // Reads the TagName string from an FGameplayTag property on a UObject.
        RC::StringType ReadRecipeTag(RC::Unreal::UObject* obj);

        // Reads the result item count from the first entry of RecipeResult,
        // purely for diagnostic completeness.
        int ReadResultCount(RC::Unreal::UObject* obj);

        // Reads a TArray<FR5BLRecipeItem> property and returns each (item path, count) pair.
        std::vector<std::pair<std::string, int>> ReadItemArray(RC::Unreal::UObject* obj, const RC::StringType& propName);
    };
}
