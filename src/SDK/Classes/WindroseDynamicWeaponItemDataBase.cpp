#include "SDK/Classes/WindroseDynamicWeaponItemDataBase.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"

using namespace RC::Unreal;

namespace Windrose {
	UClass* UWindroseDynamicWeaponItemDataBase::StaticClass()
	{
		static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Windrose.WindroseDynamicWeaponItemDataBase"));
		return Class;
	}
}