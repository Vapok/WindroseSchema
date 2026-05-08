#pragma once

#include "Unreal/Engine/UDataTable.hpp"
#include "Unreal/NameTypes.hpp"

namespace Windrose {

	struct FWindroseItemShopSettingDataRow : public RC::Unreal::FTableRowBase
	{
		RC::Unreal::FName CurrencyItemID;

		FWindroseItemShopSettingDataRow() = default;
		FWindroseItemShopSettingDataRow(const RC::StringType& InCurrency) : CurrencyItemID(RC::Unreal::FName(InCurrency, RC::Unreal::FNAME_Add)) {}
	};

}
