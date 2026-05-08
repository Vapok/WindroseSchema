#include "SDK/Classes/WindroseDynamicItemDataBase.h"

using namespace RC::Unreal;

namespace Windrose {
    bool& UWindroseDynamicItemDataBase::GetIgnoreOnSave()
    {
        auto bIgnoreOnSave = this->GetValuePtrByPropertyNameInChain<bool>(TEXT("bIgnoreOnSave"));
        return *bIgnoreOnSave;
    }
}