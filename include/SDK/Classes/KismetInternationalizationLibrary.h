#pragma once

#include "Unreal/UObject.hpp"

namespace Windrose {
	class UKismetInternationalizationLibrary : public RC::Unreal::UObject {
	public:
		static RC::Unreal::FString GetCurrentLanguage();

		static UKismetInternationalizationLibrary* GetDefaultObj();
	};
}