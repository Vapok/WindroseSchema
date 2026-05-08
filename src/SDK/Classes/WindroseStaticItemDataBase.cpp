#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "SDK/Classes/WindroseStaticItemDataBase.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
	RC::Unreal::UClass* UWindroseStaticItemDataBase::StaticClass()
	{
		static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, TEXT("/Script/Windrose.WindroseStaticItemDataBase"));
		return Class;
	}
}
