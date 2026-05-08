#include "SDK/Classes/WindroseMasterDataTables.h"
#include "Unreal/Engine/UDataTable.hpp"

namespace Windrose {
    RC::Unreal::UDataTable* UWindroseMasterDataTables::GetItemRecipeDataTable()
    {
        auto DataTable = this->GetValuePtrByPropertyNameInChain<RC::Unreal::UDataTable*>(STR("ItemRecipeDataTable"));
        if (DataTable)
        {
            return *DataTable;
        }
        return nullptr;
    }
}