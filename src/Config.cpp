#include <Windows.h>

#include "Enums.h"
#include "MxiUtils.h"

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
        return mxi::GetModuleFilePath().replace_filename("config.ini");
    }

    void Config::Load()
    {
        auto file = GetConfigFile();

        auto v = mxi::ReadIniStr(file, "Config", "PartsPerPage", "50");
        partsPerPage = atoi(v.c_str());

        v = mxi::ReadIniStr(file, "Config", "OrderPartsBy", "COLOR");
        orderPartsBy = OrderBy::FromString(v);
        v = mxi::ReadIniStr(file, "Config", "OrderSetsBy", "SETID");
        orderSetsBy = OrderBy::FromString(v);

        lastPartSearch = mxi::ReadIniStr(file, "Config", "LastPartSearch", "");
        lastSetSearch = mxi::ReadIniStr(file, "Config", "LastSetSearch", "");
        checkDate = mxi::ReadIniStr(file, "Config", "CheckDate", "");

        excludeMinifigs = mxi::ReadIniStr(file, "Config", "ExcludeMinifigs", "0") == "1";
        excludeUnchecked = mxi::ReadIniStr(file, "Config", "ExcludeUnchecked", "0") == "1";
        findMissingImages = mxi::ReadIniStr(file, "Config", "FindMissingImages", "0") == "1";
        hideFulfilled = mxi::ReadIniStr(file, "Config", "HideFulfilled", "0") == "1";
        hideIgnored = mxi::ReadIniStr(file, "Config", "HideIgnored", "0") == "1";
        findFlagged = mxi::ReadIniStr(file, "Config", "FindFlagged", "0") == "1";
        showSetInventoryRecords = mxi::ReadIniStr(file, "Config", "ShowSetInventoryRecords", "0") == "1";

        v = mxi::ReadIniStr(file, "Config", "FilterBySets", "");
        filterBySets = mxi::explode<std::string>(v);


        v = mxi::ReadIniStr(file, "Config", "FilterByParts", "");
        filterByParts = mxi::explode<std::string>(v);
    }

    void Config::Save()
    {
        auto file = GetConfigFile();

        mxi::WriteIniStr(file, "Config", "PartsPerPage", std::format("{}", partsPerPage));
        mxi::WriteIniStr(file, "Config", "OrderPartsBy", orderPartsBy->ToString());
        mxi::WriteIniStr(file, "Config", "OrderSetsBy", orderSetsBy->ToString());
        mxi::WriteIniStr(file, "Config", "LastPartSearch", lastPartSearch);
        mxi::WriteIniStr(file, "Config", "LastSetSearch", lastSetSearch);
        mxi::WriteIniStr(file, "Config", "CheckDate", checkDate);
        mxi::WriteIniStr(file, "Config", "ExcludeMinifigs", excludeMinifigs ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "ExcludeUnchecked", excludeUnchecked ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "FindMissingImages", findMissingImages ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "HideFulfilled", hideFulfilled ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "HideIgnored", hideIgnored ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "FindFlagged", findFlagged ? "1" : "0");
        mxi::WriteIniStr(file, "Config", "ShowSetInventoryRecords", showSetInventoryRecords ? "1" : "0");

        mxi::WriteIniStr(file, "Config", "FilterBySets", mxi::implode(filterBySets));
        mxi::WriteIniStr(file, "Config", "FilterByParts", mxi::implode(filterByParts));
    }
}