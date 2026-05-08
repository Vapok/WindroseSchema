#include "SDK/Classes/WindroseDynamicArmorItemDataBase.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"

using namespace RC::Unreal;

namespace Windrose {
	UClass* UWindroseDynamicArmorItemDataBase::StaticClass()
	{
		static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Windrose.WindroseDynamicArmorItemDataBase"));
		return Class;
	}
}