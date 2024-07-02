#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace jaml
{
    class Element
    {
    public:
        size_t id = 0;
        Element * parent = nullptr;
        std::vector<Element> children = {};
        std::string type = {};
        std::vector<std::pair<std::string, std::string>> attributes = {};
    };

    class Doc
    {
    public:
        Element outer;
    };

    Doc Parse(std::string_view const & source);
    Doc Parse(std::filesystem::path const & source);

}