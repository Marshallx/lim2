#include <Windows.h>

#include "Enums.h"
#include "MxUtils.h"

#include "Config.h"

namespace mx
{

    static constexpr const auto configFile = "config.ini";

    size_t Config::partsPerPage = 50;
    OrderBy * Config::orderPartsBy = &OrderBy::COLOR;
    std::string Config::lastPartSearch = {};
    std::string Config::checkDate = {};
    bool Config::excludeMinifigs = false;
    bool Config::excludeUnchecked = false;
    bool Config::findMissingImages = false;
    bool Config::hideFulfilled = false;
    bool Config::hideIgnored = false;
    bool Config::findFlagged = false;
    bool Config::showSetInventoryRecords = false;
    std::vector<std::string> Config::filterBySets = {};
    std::vector<std::string> Config::filterByParts = {};

    OrderBy * Config::orderSetsBy = &OrderBy::SETID;
    std::string Config::lastSetSearch = {};

    std::filesystem::path Config::GetConfigFile()
    {
        return MxUtils::GetModuleFilePath().replace_filename("config.ini");
    }

    void Config::Load()
    {
        auto file = GetConfigFile();

        auto v = MxUtils::ReadIniStr(file, "Config", "PartsPerPage", "50");
        partsPerPage = atoi(v.c_str());

        v = MxUtils::ReadIniStr(file, "Config", "OrderPartsBy", "COLOR");
        orderPartsBy = OrderBy::FromString(v);
        v = MxUtils::ReadIniStr(file, "Config", "OrderSetsBy", "SETID");
        orderSetsBy = OrderBy::FromString(v);

        lastPartSearch = MxUtils::ReadIniStr(file, "Config", "LastPartSearch", "");
        lastSetSearch = MxUtils::ReadIniStr(file, "Config", "LastSetSearch", "");
        checkDate = MxUtils::ReadIniStr(file, "Config", "CheckDate", "");

        excludeMinifigs = MxUtils::ReadIniStr(file, "Config", "ExcludeMinifigs", "0") == "1";
        excludeUnchecked = MxUtils::ReadIniStr(file, "Config", "ExcludeUnchecked", "0") == "1";
        findMissingImages = MxUtils::ReadIniStr(file, "Config", "FindMissingImages", "0") == "1";
        hideFulfilled = MxUtils::ReadIniStr(file, "Config", "HideFulfilled", "0") == "1";
        hideIgnored = MxUtils::ReadIniStr(file, "Config", "HideIgnored", "0") == "1";
        findFlagged = MxUtils::ReadIniStr(file, "Config", "FindFlagged", "0") == "1";
        showSetInventoryRecords = MxUtils::ReadIniStr(file, "Config", "ShowSetInventoryRecords", "0") == "1";

        v = MxUtils::ReadIniStr(file, "Config", "FilterBySets", "");
        filterBySets = MxUtils::Explode(v);


        v = MxUtils::ReadIniStr(file, "Config", "FilterByParts", "");
        filterByParts = MxUtils::Explode(v);
    }

    void Config::Save()
    {
        auto file = GetConfigFile();

        MxUtils::WriteIniStr(file, "Config", "PartsPerPage", std::format("{}", partsPerPage));
        MxUtils::WriteIniStr(file, "Config", "OrderPartsBy", orderPartsBy->ToString());
        MxUtils::WriteIniStr(file, "Config", "OrderSetsBy", orderSetsBy->ToString());
        MxUtils::WriteIniStr(file, "Config", "LastPartSearch", lastPartSearch);
        MxUtils::WriteIniStr(file, "Config", "LastSetSearch", lastSetSearch);
        MxUtils::WriteIniStr(file, "Config", "CheckDate", checkDate);
        MxUtils::WriteIniStr(file, "Config", "ExcludeMinifigs", excludeMinifigs ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "ExcludeUnchecked", excludeUnchecked ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "FindMissingImages", findMissingImages ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "HideFulfilled", hideFulfilled ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "HideIgnored", hideIgnored ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "FindFlagged", findFlagged ? "1" : "0");
        MxUtils::WriteIniStr(file, "Config", "ShowSetInventoryRecords", showSetInventoryRecords ? "1" : "0");

        MxUtils::WriteIniStr(file, "Config", "FilterBySets", MxUtils::Implode(filterBySets));
        MxUtils::WriteIniStr(file, "Config", "FilterByParts", MxUtils::Implode(filterByParts));
    }
}