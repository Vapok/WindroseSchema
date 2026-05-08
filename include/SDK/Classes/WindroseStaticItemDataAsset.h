#pragma once

#include "SDK/Classes/UDataAsset.h"
#include "SDK/Classes/WindroseStaticItemDataBase.h"
#include "Unreal/Core/Containers/Map.hpp"

namespace Windrose {
	class UWindroseStaticItemDataAsset : public UECustom::UDataAsset {
	public:
		RC::Unreal::TMap<RC::Unreal::FName, UWindroseStaticItemDataBase*> StaticItemDataMap;
	};
}