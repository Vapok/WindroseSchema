#include "SDK/Classes/WindroseStaticWeaponItemData.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
	UClass* UWindroseStaticWeaponItemData::StaticClass()
	{
		static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Windrose.WindroseStaticWeaponItemData"));
		return Class;
	}
}