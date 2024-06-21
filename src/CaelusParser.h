#pragma once

#include <string_view>

#include "CaelusClass.h"
#include "CaelusWindow.h"

namespace Caelus
{
    class CaelusParser
    {
    public:
        CaelusParser(std::string_view const & source, CaelusClassMap & classes);

    private:
        std::vector<std::string_view> lines = {};
        std::string_view linetext;
        size_t col = 0;
        size_t lineno = 0;

        void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Expected(std::string_view const & expected) const;
        void Expected(char const expected) const;
        void EoiCheck(std::string_view const & expected) const;
        void EoiCheck(char const expected) const;
        char Peek() const;
        bool IsWhitespace() const;
        void EatWhitespace();
        void NextLine();
        std::string_view ParseKey();
        std::string_view ParseValue();
        void ParseSection(CaelusClassMap & classes);


    };
}