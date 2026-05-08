#pragma once

#include "Unreal/UObject.hpp"
#include "SDK/Classes/WindroseStaticItemDataBase.h"

namespace Windrose {
	class UWindroseStaticWeaponItemData : public UWindroseStaticItemDataBase {
	public:
		static RC::Unreal::UClass* StaticClass();
	};
}