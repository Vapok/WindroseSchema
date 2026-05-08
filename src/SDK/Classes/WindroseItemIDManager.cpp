#include "SDK/Classes/WindroseItemIDManager.h"
#include "SDK/Classes/WindroseStaticItemDataTable.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include <Unreal/CoreUObject/UObject/Class.hpp>
#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    UWindroseStaticItemDataTable* UWindroseItemIDManager::GetStaticItemDataTable()
    {
        auto StaticItemDataTable = this->GetValuePtrByPropertyNameInChain<UWindroseStaticItemDataTable*>(STR("StaticItemDataTable"));
        if (StaticItemDataTable)
        {
            return *StaticItemDataTable;
        }
        return nullptr;
    }

    UObject* UWindroseItemIDManager::GetStaticItemData(const FName& StaticItemId)
    {
        static auto Function = UECustom::UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Script/Windrose.WindroseItemIDManager:GetStaticItemData"));

        if (!Function)
        {
            PS::Log<LogLevel::Error>(STR("/Script/Windrose.WindroseItemIDManager:GetStaticItemData not found, unable to call UWindroseItemIDManager::GetStaticItemData\n"));
            return nullptr;
        }

        struct LocalParams {
            FName StaticItemId;
            UObject* ReturnValue;
        }params;

        params.StaticItemId = StaticItemId;

        this->ProcessEvent(Function, &params);

        return params.ReturnValue;
    }
}