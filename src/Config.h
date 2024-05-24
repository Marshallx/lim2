#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Enums.h"

namespace mx
{
    class Config
    {
    private:
        static std::filesystem::path GetConfigFile();

    public:
        static void Load();
        static void Save();

        static size_t partsPerPage;
        static OrderBy * orderPartsBy;
        static std::string lastPartSearch;
        static std::string checkDate;
        static bool excludeMinifigs;
        static bool excludeUnchecked;
        static bool findMissingImages;
        static bool hideFulfilled;
        static bool hideIgnored;
        static bool findFlagged;
        static bool showSetInventoryRecords;
        static std::vector<std::string> filterBySets;
        static std::vector<std::string> filterByParts;

        static size_t setsPerPage;
        static OrderBy * orderSetsBy;
        static std::string lastSetSearch;

    };
}