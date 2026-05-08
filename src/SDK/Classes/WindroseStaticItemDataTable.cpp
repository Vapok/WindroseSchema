#include "SDK/Classes/WindroseStaticItemDataTable.h"
#include "SDK/Classes/WindroseStaticItemDataAsset.h"
#include "Helpers/Casting.hpp"

using namespace RC;

namespace Windrose {
    UWindroseStaticItemDataAsset* UWindroseStaticItemDataTable::GetDataAsset()
    {
        auto DataAsset = *Helper::Casting::ptr_cast<UWindroseStaticItemDataAsset**>(this, 0x28);
        return DataAsset;
    }
}