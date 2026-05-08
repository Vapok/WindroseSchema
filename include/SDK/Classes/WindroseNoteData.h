#pragma once

#include "Unreal/UObject.hpp"

namespace RC::Unreal {
    class UClass;
}

namespace Windrose {
	class UWindroseNoteData : public RC::Unreal::UObject {
	public:
        static RC::Unreal::UClass* StaticClass();
	};
}
