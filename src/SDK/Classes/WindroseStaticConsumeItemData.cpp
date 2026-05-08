#include "SDK/Classes/WindroseStaticConsumeItemData.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
	UClass* UWindroseStaticConsumeItemData::StaticClass()
	{
		static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Windrose.WindroseStaticConsumeItemData"));
		return Class;
	}
}