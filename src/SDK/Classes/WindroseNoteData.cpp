#include "SDK/Classes/WindroseNoteData.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"

using namespace RC::Unreal;

namespace Windrose {
    UClass* UWindroseNoteData::StaticClass()
    {
        static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, TEXT("/Script/Windrose.WindroseNoteData"));
        return Class;
    }
}