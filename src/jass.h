#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>

namespace jass
{
    class Style
    {
    public:
        Style(std::string_view const & prop, std::string_view const & value, size_t line, size_t col) : prop(std::string{ prop }), value(std::string{ value }), line(line), col(col));
        std::string prop;
        std::string value;
        size_t line;
        size_t col;
    };

    class Rule
    {
    public:
        std::string selectors;
        std::unordered_map<char const *, Style> styles{};
    };

    std::vector<Rule> Parse(std::string_view const & source);
    std::vector<Rule> Parse(std::filesystem::path const & source);

}