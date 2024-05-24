#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mx
{
    namespace MxUtils
    {
        // Convert UTF-16 std::wstring to UTF-8 std::string.
        std::string Utf8String(std::wstring const & utf16);

        // Convert UTF-8 std::string to UTF-16 std::wstring.
        std::wstring Utf16String(std::string const & utf8);

        std::string ReadIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & defaultValue = {});

        void WriteIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & value);

        // Split a string into a vector of strings.
        std::vector<std::string> Explode(std::string_view const & s, std::string_view const & delim = ",");

        // Join a vector of strings into a single string.
        std::string Implode(std::vector<std::string> const & v, std::string_view const & delim = ",");

        std::filesystem::path GetModuleFilePath();

        // Check if a character requires escaping for JSON.
        bool json_escape_needed(unsigned char const c);

        // Check if a string requires escaping for JSON (if false then just needs wrapping in quotemarks)
        bool json_escape_needed(std::string const & s);

        // JSON-escape strings in-place without wrapping in quotemarks
        void json_escape_string(std::string & s);

        // Unescape a JSON string. Must start and end with quotemarks. Ignores characters after closing quotemark, optionally returns number of characters parsed from source.
        std::string json_unescape_string(std::string_view const & s, size_t * cchParsed);
    }
}