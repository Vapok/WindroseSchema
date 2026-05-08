#pragma once

#include "SDK/Classes/UDataAsset.h"
#include "SDK/Classes/WindroseNoteData.h"
#include "Unreal/Core/Containers/Map.hpp"

namespace Windrose {
	class UWindroseNoteDataAsset : public UECustom::UDataAsset {
	public:
		RC::Unreal::TMap<RC::Unreal::FName, UWindroseNoteData*> NoteDataMap;
	};
}
