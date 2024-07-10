#pragma once

#include <filesystem>
#include <string_view>
#include "CaelusWindow.h"

namespace Caelus
{

    class JamlParser
    {
    public:
        JamlParser(std::string_view const & source, CaelusWindow & window);

    private:
        std::string_view const & source;
        CaelusElement & e;
        char c = 0;
        size_t size;
        size_t pos = 0;
        size_t col = 0;
        size_t line = 0;

        [[noreturn]] void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Eat(std::string_view const & expected);
        void RememberPos(bool const stash = true);
        bool LookAhead(std::string_view const & expected, bool const eatIfFound = false);
        void EatWhitespace();
        void EatCommentsAndWhitespace();
        char NextChar();
        std::string_view ParseName();
        std::string_view ParseKey();
        std::string_view ParseValue();
        void ParseAttributes();
        void ParseContent();
        void ParseTag();
    };

    // Unescape HTML text (e.g. &amp; to &)
    std::string unescape(std::string_view const & s);
}