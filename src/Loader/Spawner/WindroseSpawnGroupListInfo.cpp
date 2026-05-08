#include "Loader/Spawner/WindroseSpawnGroupListInfo.h"
#include "nlohmann/json.hpp"
#include "Utility/JsonHelpers.h"

using namespace RC;
using namespace RC::Unreal;

namespace PS {
    void WindroseSpawnGroupListInfo::AddWindroseListInfo(const nlohmann::json& value)
    {
        if (!value.is_object())
        {
            throw std::runtime_error("WindroseList item was not an object.");
        }

        Windrose::WindroseListInfo listInfo;
        if (PS::JsonHelpers::FieldExists(value, "NPCID"))
        {
            PS::JsonHelpers::ParseFName(value, "NPCID", listInfo.NPCID);
        }

        if (PS::JsonHelpers::FieldExists(value, "WindroseId"))
        {
            PS::JsonHelpers::ParseFName(value, "WindroseId", listInfo.WindroseId);
        }

        if (listInfo.NPCID == NAME_None && listInfo.WindroseId == NAME_None)
        {
            throw std::runtime_error("WindroseList item must contain either NPCID or WindroseId.");
        }

        if (PS::JsonHelpers::FieldExists(value, "Level"))
        {
            PS::JsonHelpers::ParseInteger(value, "Level", listInfo.Level);
        }

        if (PS::JsonHelpers::FieldExists(value, "Level_Max"))
        {
            PS::JsonHelpers::ParseInteger(value, "Level_Max", listInfo.LevelMax);
        }

        if (PS::JsonHelpers::FieldExists(value, "Num"))
        {
            PS::JsonHelpers::ParseInteger(value, "Num", listInfo.Num);
        }

        if (PS::JsonHelpers::FieldExists(value, "Num_Max"))
        {
            PS::JsonHelpers::ParseInteger(value, "Num_Max", listInfo.NumMax);
        }

        WindroseList.push_back(listInfo);
    }
}