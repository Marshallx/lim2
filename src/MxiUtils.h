#pragma once

#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

#define MX_THROW(message) { throw std::runtime_error(mxi::formatError(message).str()); }

namespace mxi
{
    std::filesystem::path GetModuleFilePath();

    // Convert UTF-16 std::wstring to UTF-8 std::string.
    std::string Utf8String(std::wstring const & utf16);

    // Convert UTF-8 std::string to UTF-16 std::wstring.
    std::wstring Utf16String(std::string const & utf8);

    std::string ReadIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & defaultValue = {});

    void WriteIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & value);

    std::string create_guid();

    // Split a string into a vector of string_views (default) or strings (use explode<std::string>()).
    template<typename S = std::string_view>
    std::vector<S> explode(std::string_view const & s, std::string_view const & delim = ",")
    {
        auto vs = std::vector<S>{};
        auto pos = size_t{};
        for (auto fd = size_t{ 0 }; (fd = s.find(delim, pos)) != std::string::npos; pos = fd + delim.size())
        {
            vs.emplace_back(s.data() + pos, s.data() + fd);
        }
        vs.emplace_back(s.data() + pos, s.data() + s.size());
        return vs;
    }

    // Join a vector of strings (or string_views) into a single string.
    template<typename S>
    std::string implode(std::vector<S> const & vs, std::string_view const & delim = ",")
    {
        auto s = std::string{};
        for (auto const & v : vs)
        {
            s.append(v);
            s.append(delim);
        }
        s.pop_back();
        return s;
    }

    std::ostringstream formatError(std::string_view const & message, std::source_location const && source = {});

    // Check if a character requires escaping for JSON.
    bool json_escape_needed(unsigned char const c);

    // Check if a string requires escaping for JSON (if false then just needs wrapping in quotemarks)
    bool json_escape_needed(std::string const & s);

    // JSON-escape strings in-place without wrapping in quotemarks
    void json_escape_string(std::string & s);

    // Unescape a JSON string. Must start and end with quotemarks. Ignores characters after closing quotemark, optionally returns number of characters parsed from source.
    std::string json_unescape_string(std::string_view const & s, size_t * cchParsed);

    // Gets the portion of the string sans leading and trailing whitespace ( \t\r\n)
    [[nodiscard]] std::string_view trim(std::string_view const & str);

}