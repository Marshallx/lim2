#pragma once

#include <filesystem>
#include "sqlite3/sqlite3.h"

namespace mx
{
    class Database
    {
    public:
        void Open(std::filesystem::path const & path);
        int GetVersion();

    private:
        void CheckResult(int r, int expected = SQLITE_OK);
        sqlite3 * dbh_ = nullptr;
    };
}