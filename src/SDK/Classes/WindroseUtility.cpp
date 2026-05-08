#include "SDK/Classes/WindroseUtility.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include <Unreal/CoreUObject/UObject/Class.hpp>
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    UWindroseMasterDataTables* UWindroseUtility::GetMasterDataTables(UObject* WorldContextObject)
    {
        static auto Function = UECustom::UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Script/Windrose.WindroseUtility:GetMasterDataTables"));

        if (!Function)
        {
            PS::Log<LogLevel::Error>(STR("/Script/Windrose.WindroseUtility:GetMasterDataTables not found, unable to call UWindroseUtility::GetMasterDataTables\n"));
            return nullptr;
        }

        struct LocalParams {
            UObject* WorldContextObject;
            UWindroseMasterDataTables* ReturnValue;
        }params;

        params.WorldContextObject = WorldContextObject;

        GetDefaultObject()->ProcessEvent(Function, &params);

        return params.ReturnValue;
    }

    void UWindroseUtility::Alert(UObject* WorldContextObject, const FText& Message)
    {
        static auto Function = UECustom::UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Script/Windrose.WindroseUtility:Alert"));

        if (!Function)
        {
            PS::Log<LogLevel::Error>(STR("/Script/Windrose.WindroseUtility:Alert not found, unable to call UWindroseUtility::Alert\n"));
            return;
        }

        struct LocalParams {
            UObject* WorldContextObject;
            FText Message;
        }params;

        params.WorldContextObject = WorldContextObject;
        params.Message = Message;

        GetDefaultObject()->ProcessEvent(Function, &params);
    }

    UWindroseItemIDManager* UWindroseUtility::GetItemIDManager(RC::Unreal::UObject* WorldContextObject)
    {
        static auto Function = UECustom::UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Script/Windrose.WindroseUtility:GetItemIDManager"));

        if (!Function)
        {
            PS::Log<LogLevel::Error>(STR("/Script/Windrose.WindroseUtility:GetItemIDManager not found, unable to call UWindroseUtility::GetItemIDManager\n"));
            return nullptr;
        }

        struct LocalParams {
            UObject* WorldContextObject;
            UWindroseItemIDManager* ReturnValue;
        }params;

        params.WorldContextObject = WorldContextObject;

        GetDefaultObject()->ProcessEvent(Function, &params);

        return params.ReturnValue;
    }

    UWindroseUtility* UWindroseUtility::GetDefaultObject()
    {
        static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Windrose.WindroseUtility"));
        return static_cast<UWindroseUtility*>(Class->GetClassDefaultObject());
    }
}