#pragma once

#include "Unreal/Engine/UDataTable.hpp"
#include "Unreal/Core/Containers/Array.hpp"
#include "Unreal/NameTypes.hpp"

namespace Windrose {
	struct FWindroseItemShopLotteryEntry
	{
		RC::Unreal::FName ShopGroupName;
		int32_t Weight;
	};

	struct FWindroseItemShopLotteryDataRow : public RC::Unreal::FTableRowBase
	{
		RC::Unreal::TArray<FWindroseItemShopLotteryEntry> lotteryDataArray;
	};
}
