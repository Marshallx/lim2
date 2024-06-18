#pragma once

#include <string_view>

#include "CaelusClass.h"
#include "CaelusWindow.h"

namespace Caelus
{
    class SourceLoc
    {
    public:
        size_t pos = 0;
        size_t line = 1;
        size_t col = 1;
    };

    class CaelusParser
    {
    public:
        CaelusParser(std::string_view const & source, CaelusClassMap & classes);

    private:
        std::string_view source;
        SourceLoc loc;

        void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Expected(std::string_view const & expected) const;
        void Expected(char const expected) const;
        void EoiCheck(std::string_view const & expected) const;
        void EoiCheck(char const expected) const;
        void NextChar();
        char Peek() const;
        bool IsWhitespace() const;
        void EatWhitespace(bool const eatLF = true);
        void EatComments();
        std::string_view ParseKey();
        std::string ParseValue();
        void ParseSection(CaelusClassMap & classes);


    };
}