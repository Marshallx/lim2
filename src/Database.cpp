#include <stdexcept>

#include <Windows.h>

#include "MxiUtils.h"

#include "Database.h"

namespace mx
{
    void Database::Open(std::filesystem::path const & path)
    {
        auto const r = sqlite3_open(path.string().c_str(), &dbh_);
        CheckResult(r);
    }

    int Database::GetVersion()
    {
        sqlite3_stmt * stmt = nullptr;
        auto r = sqlite3_prepare_v2(dbh_, "SELECT value FROM meta where key=:key LIMIT 1", -1, &stmt, nullptr);
        CheckResult(r);
        r = sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":key"), "version", -1, SQLITE_STATIC);
        CheckResult(r);
        r = sqlite3_step(stmt);
        CheckResult(r, SQLITE_ROW);
        auto const value = sqlite3_column_text(stmt, 0);
        auto const version = atoi(reinterpret_cast<const char *>(value));
        r = sqlite3_finalize(stmt);
        CheckResult(r);
        return version;
    }

    void Database::CheckResult(int r, int expected)
    {
        if (r == expected) return;
        auto msg = std::format("Sqlite error: {}/{}\n{}", r, sqlite3_extended_errcode(dbh_), sqlite3_errmsg(dbh_));
        MessageBox(NULL, mxi::Utf16String(msg).c_str(), L"", MB_OK);
        throw std::runtime_error(msg.c_str());
    }
}