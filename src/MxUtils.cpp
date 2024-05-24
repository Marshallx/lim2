#include <deque>

#include <Windows.h>

#include "MxErr.h"

#include "MxUtils.h"

namespace mx
{
    namespace MxUtils
    {
        using namespace mx::err;

        std::string Utf8String(std::wstring const & utf16)
        {
            auto size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), NULL, 0, NULL, NULL);
            auto utf8 = std::string{};
            utf8.resize(size);
            WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), utf8.data(), static_cast<int>(utf8.size()), NULL, NULL);
            return utf8;
        }

        std::wstring Utf16String(std::string const & utf8)
        {
            auto size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
            auto utf16 = std::wstring{};
            utf16.resize(size);
            MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), utf16.data(), static_cast<int>(utf16.size()));
            return utf16;
        }

        std::string ReadIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & defaultValue)
        {
            auto section16 = Utf16String(section);
            auto key16 = Utf16String(key);
            auto default16 = Utf16String(defaultValue);

            auto value16 = std::wstring{};
            value16.resize(100);

            for (;;)
            {
                auto copied = GetPrivateProfileString(section16.c_str(), key16.c_str(), default16.c_str(), value16.data(), static_cast<int>(value16.size()), path.wstring().c_str());
                if (copied == value16.size() - 1)
                {
                    value16.resize(value16.size() * 2);
                    continue;
                }
                value16.resize(copied);
                break;
            }

            return Utf8String(value16);
        }

        void WriteIniStr(std::filesystem::path const & path, std::string const & section, std::string const & key, std::string const & value)
        {
            auto section16 = Utf16String(section);
            auto key16 = Utf16String(key);
            auto value16 = Utf16String(value);

            WritePrivateProfileString(section16.c_str(), key16.c_str(), value16.c_str(), path.wstring().c_str());
        }

        std::vector<std::string> Explode(std::string_view const & s, std::string_view const & delim)
        {
            auto vs = std::vector<std::string>{};
            auto pos = size_t{};
            for (auto fd = size_t{ 0 }; (fd = s.find(delim, pos)) != std::string::npos; pos = fd + delim.size())
            {
                vs.emplace_back(s.data() + pos, s.data() + fd);
            }
            vs.emplace_back(s.data() + pos, s.data() + s.size());
            return vs;
        }

        std::string Implode(std::vector<std::string> const & vs, std::string_view const & delim)
        {
            auto s = std::string{};
            for (auto v : vs)
            {
                s.append(v);
                s.append(delim);
            }
            s.pop_back();
            return s;
        }

        std::filesystem::path GetModuleFilePath()
        {
            auto selfexe = std::wstring{};
            selfexe.resize(MAX_PATH);
            GetModuleFileNameW(0, selfexe.data(), static_cast<DWORD>(selfexe.size()));
            return std::filesystem::path(selfexe);
        }

        bool json_escape_needed(unsigned char const c)
        {
            return (c < 0x1F || c == 0x7F || c == '"' || c == '\\');
        }

        bool json_escape_needed(std::string const & s)
        {
            for (auto & c : s) { if (json_escape_needed(c)) { return true; } }
            return false;
        }

        // Simple JSON-escape strings in-place without wrapping in quotemarks
        void json_escape_string(std::string & s)
        {
            if (!json_escape_needed(s)) return;

            auto const inputLen = s.size();
            auto q = std::deque<std::string::value_type>();

            for (auto i = decltype(inputLen)(0); i < inputLen; ++i)
            {
                auto const c = s[i];
                if (!json_escape_needed(c))
                {
                    q.push_back(c);
                }
                else
                {
                    q.push_back('\\');
                    switch (c)
                    {
                    case '"':  q.push_back('"'); break;
                    case '\\': q.push_back('\\'); break;
                    case '\b': q.push_back('b'); break;
                    case '\f': q.push_back('f'); break;
                    case '\n': q.push_back('n'); break;
                    case '\r': q.push_back('r'); break;
                    case '\t': q.push_back('t'); break;
                    default:
                        auto const & u = std::format("u{:04x}", (unsigned char)(c));
                        q.insert(std::end(q), std::begin(u), std::end(u));
                        break;
                    }
                }
                s[i] = q.front();
                q.pop_front();
            }

            while (!q.empty())
            {
                s += q.front();
                q.pop_front();
            }
        }

        std::string json_unescape_string(std::string_view const & s, size_t * cchParsed = nullptr)
        {
            size_t pos;
            if (!cchParsed) cchParsed = &pos;
            if (s.empty() || s[0] != '"') MX_THROW("Expected a quoted JSON string.");
            auto oss = std::ostringstream{};
            for (*cchParsed = 0; *cchParsed < s.size(); ++(*cchParsed))
            {
                switch (s[*cchParsed])
                {
                case '"':
                    ++(*cchParsed);
                    return oss.str();

                case ('\\'):
                    ++(*cchParsed);
                    if (*cchParsed >= s.size()) MX_THROW(std::format("Unexpected end of JSON string at position {}.", *cchParsed - 1))
                    switch (s[*cchParsed])
                    {
                    case '"':  oss << '"'; break;
                    case '\\': oss << '\\'; break;
                    case 'b': oss << '\b'; break;
                    case 'f': oss << '\f'; break;
                    case 'n': oss << '\n'; break;
                    case 'r': oss << '\r'; break;
                    case 't': oss << '\t'; break;
                    case 'u':
                        // TODO
                        break;
                    default:
                        MX_THROW(std::format("Unrecognized JSON escape sequence \\{} at position {}.", s[*cchParsed], *cchParsed - 1));
                    }
                default:
                    oss << s[*cchParsed];
                    break;
                }
            }
            MX_THROW(std::format("Unexpected end of JSON string at position {}.", *cchParsed - 1));
        }
    }
}