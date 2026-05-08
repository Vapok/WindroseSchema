#pragma once

#include "Unreal/UObject.hpp"
#include "Unreal/FText.hpp"

namespace Windrose {
    class UWindroseMasterDataTables;
    class UWindroseItemIDManager;

    class UWindroseUtility : public RC::Unreal::UObject {
    public:
        static UWindroseMasterDataTables* GetMasterDataTables(RC::Unreal::UObject* WorldContextObject);

        static void Alert(RC::Unreal::UObject* WorldContextObject, const RC::Unreal::FText& Message);

        static UWindroseItemIDManager* GetItemIDManager(RC::Unreal::UObject* WorldContextObject);
    private:
        static UWindroseUtility* GetDefaultObject();
    };
}