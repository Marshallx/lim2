#pragma once

#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

#define MX_THROW(message) { throw std::runtime_error(mxi::formatError(message).str()); }

/*
    // Declaration
    MX_COLLECTION(Fruit, Apple, Orange, Banana);
    //public:
        bool IsOrange() const;
    private:
        bool IsYellow() const;
    }; // <-- required

    // Definitions
    MX_MEMBER(Fruit, Apple);
    MX_MEMBER(Fruit, Orange);
    MX_MEMBER(Fruit, Banana);
    bool IsOrange() const { return value == Value::Orange; }
    bool IsYallow() const { return value == Value::Banana; }
*/

#define MX_COLLECTION(COLLECTION_NAME, ...) \
    class COLLECTION_NAME \
    { \
    public: \
        enum class Value : uint8_t \
        { \
            __VA_ARGS__ \
        }; \
        constexpr operator Value() const { return value; } \
        explicit operator bool() const = delete; \
        constexpr bool operator==(COLLECTION_NAME other) const { return value == other.value; } \
        constexpr bool operator!=(COLLECTION_NAME other) const { return value != other.value; } \
    private: \
        Value value; \
        COLLECTION_NAME() = delete; \
        COLLECTION_NAME(COLLECTION_NAME const &) = delete; \
        COLLECTION_NAME(COLLECTION_NAME const &&) = delete; \
        constexpr COLLECTION_NAME(Value v) : value(v) { } \
    public: \
        static COLLECTION_NAME __VA_ARGS__;

#define MX_MEMBER(COLLECTION_NAME, MEMBER_NAME) \
    COLLECTION_NAME COLLECTION_NAME::MEMBER_NAME = { Value::MEMBER_NAME };

namespace mxi
{

    template <typename>
    constexpr auto always_false = false;

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
    std::vector<S> explode(std::string_view const & s, std::string_view const & delim = ",", size_t limit = std::string::npos)
    {
        auto vs = std::vector<S>{};
        auto pos = size_t{};
        auto count = size_t{};
        if (count > 1)
        {
            for (auto fd = size_t{ 0 }; (fd = s.find(delim, pos)) != std::string::npos; pos = fd + delim.size())
            {
                vs.emplace_back(s.data() + pos, s.data() + fd);
                if (++count >= limit) break;
            }
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

    // Check if a string requires escaping for JSON (if false then just needs wrapping in quotemarks).
    bool json_escape_needed(std::string const & s);

    // JSON-escape strings in-place without wrapping in quotemarks.
    void json_escape_string(std::string & s);

    // Unescape a JSON string. Must start and end with quotemarks. Ignores characters after closing quotemark, optionally returns number of characters parsed from source.
    std::string json_unescape_string(std::string_view const & s, size_t * cchParsed);

    // Gets the portion of the string sans leading and trailing whitespace ( \t\r\n).
    [[nodiscard]] std::string_view trim(std::string_view const & str);

    // Read entire file into memory.
    std::string file_get_contents(std::filesystem::path const & path);

}