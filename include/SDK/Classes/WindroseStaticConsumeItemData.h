#pragma once

#include "Unreal/UObject.hpp"
#include "SDK/Classes/WindroseStaticItemDataBase.h"

namespace Windrose {
	class UWindroseStaticConsumeItemData : public UWindroseStaticItemDataBase {
	public:
		static RC::Unreal::UClass* StaticClass();
	};
}