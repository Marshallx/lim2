#include <format>

#include "MxiLogging.h"
#include "MxiUtils.h"

#include "jass.h"

namespace jass
{
    class JassParser
    {
    public:
        JassParser(std::string_view const & source, std::vector<Rule> & rules);
        std::string_view const & source;
        char c = 0;
        size_t size;
        size_t pos = 0;
        size_t col = 0;
        size_t line = 0;

        void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Eat(std::string_view const & expected);
        void RememberPos(bool const stash = true);
        bool LookAhead(std::string_view const & expected, bool const eatIfFound = false);
        void EatWhitespace();
        void EatCommentsAndWhitespace();
        char NextChar();
        const char * ValidateProperty(std::string_view const & k) const;
        std::string_view ParseSelectors();
        std::string_view ParseKey();
        std::string_view ParseValue();
        Rule ParseRule();
    };

    void JassParser::RememberPos(bool const stash)
    {
        static size_t savedPos = 0;
        static size_t savedCol = 0;
        static size_t savedLine = 0;
        if (stash)
        {
            savedPos = pos;
            savedCol = col;
            savedLine = line;
            return;
        }
        pos = savedPos;
        col = savedCol;
        line = savedLine;
        c = source[pos];
    }

    void JassParser::Error(std::string_view const & msg) const
    {
        MX_THROW(std::format("JASS parse error at {},{}: {}", line, col, msg));
    }

    void JassParser::Expect(char const expected) const
    {
        if (c == expected) return;
        Error(std::format("Expected '{}'", expected));
    }

    void JassParser::Eat(std::string_view const & expected)
    {
        auto end = pos + expected.size();
        if (end < size)
        {
            auto const peek = std::string_view{
                source.begin() + pos, source.begin() + end
            };
            if (peek == expected)
            {
                pos = end;
                return;
            }
        }
        Error(std::format("Expected \"{}\"", expected));
    }

    void JassParser::EatWhitespace()
    {
        while (c)
        {
            switch (c)
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                NextChar();
                continue;
            default:
                return;
            }
        }
    }

    bool JassParser::LookAhead(std::string_view const & expected, bool eatIfFound)
    {
        auto const start = pos + 1;
        auto const end = start + expected.size();
        if (end >= size) return false;
        auto look = std::string_view{
            source.begin() + start, source.begin() + end
        };
        if (look != expected) return false;
        if (eatIfFound) Eat(expected);
        return true;
    }

    void JassParser::EatCommentsAndWhitespace()
    {
        for (;;)
        {
            EatWhitespace();
            if (c != '/') return;
            if (!LookAhead("*")) return;
            auto const sub = source.substr(pos + 2);
            auto n = sub.find("*/");
            if (n == std::string::npos) Error("Unterminated comment.");
            n += 2;
            while (pos != n) NextChar();
        }
    }

    char JassParser::NextChar()
    {
        if (c == '\n')
        {
            ++line;
            col = 0;
        }
        else
        {
            ++col;
        }
        ++pos;
        c = (pos >= size) ? 0 : source[pos];
        return c;
    }

    JassParser::JassParser(std::string_view const & source, std::vector<Rule> & rules) : source(source)
    {
        size = source.size();
        if (!size) Error("Empty document");
        c = source[0];
        for(;;)
        {
            EatCommentsAndWhitespace();
            if (!c) break;
            rules.push_back(ParseRule());
        }
    }

    std::string_view JassParser::ParseSelectors()
    {
        auto start = pos;
        while (c != '{') NextChar();
        return { source.begin() + start, source.begin() + pos };
    }

    std::string_view JassParser::ParseKey()
    {
        for (auto start = pos; ; NextChar())
        {
            switch (c)
            {
            case 0:
            case ' ':
            case ':':
            case '\t':
            case '\r':
            case '\n':
            case '}':
            case '/':
                return { source.begin() + start, source.begin() + pos };
            }
        }
    }

    std::string_view JassParser::ParseValue()
    {
        Eat(":");
        EatCommentsAndWhitespace();
        for (auto start = pos; ; NextChar())
        {
            switch (c)
            {
            case 0:
            case '\n':
            case ';':
            case '}':
                return { source.begin() + start, source.begin() + pos };
            }
        }
    }


    Rule JassParser::ParseRule()
    {
        auto rule = Rule{};
        rule.selectors = ParseSelectors();
        EatCommentsAndWhitespace();
        Eat("{");
        EatCommentsAndWhitespace();
        while (c)
        {
            if (c == '}') break;
            auto k = ParseKey();
            auto p = ValidateProperty(k);
            EatCommentsAndWhitespace();
            auto v = ParseValue();
            rule.styles[p] = { k, v, line, col };
            if (c != '}') NextChar();
            EatCommentsAndWhitespace();
        }
        return rule;
    }

    const char * JassParser::ValidateProperty(std::string_view const & k) const
    {
        static constexpr auto names = {
            "background-color",
            "border",
            "border-bottom",
            "border-left",
            "border-right",
            "border-top",
            "bottom",
            "color",
            "font-face",
            "font-size",
            "height",
            "left",
            "max-width",
            "min-width",
            "padding",
            "padding-bottom",
            "padding-left",
            "padding-right",
            "padding-top",
            "right",
            "top",
            "width"
        };

        for (auto const & name : names)
        {
            if (k == name) return name;
        }

        Error(std::format("Unknown JASS property \"{}\"", k));
    }
    /*
        try
        {
            if (key == "parent") c->SetParentName(value);
            else if (key == "background-color") c->SetBackgroundColor(value);
            else if (key == "border") c->SetBorder(value);
            else if (key == "border-bottom") c->SetBorder(value, BOTTOM);
            else if (key == "border-left") c->SetBorder(value, LEFT);
            else if (key == "border-right") c->SetBorder(value, RIGHT);
            else if (key == "border-top") c->SetBorder(value, TOP);
            else if (key == "bottom") c->SetTether(BOTTOM, value);
            else if (key == "class") c->AddClassNames(value);
            else if (key == "text-color" || key == "color") c->SetTextColor(value);
            else if (key == "font-face") c->SetFontFace(value);
            else if (key == "font-size") c->SetFontSize(value);
            else if (key == "height") c->SetHeight(value);
            else if (key == "label") c->SetLabel(value);
            else if (key == "left") c->SetTether(LEFT, value);
            //TODO else if (key == "max-width") c->SetMaxWidth(value);
            //TODO else if (key == "min-width") c->SetMinWidth(value);
            else if (key == "padding-bottom") c->SetPadding(value, BOTTOM);
            else if (key == "padding-left") c->SetPadding(value, LEFT);
            else if (key == "padding-right") c->SetPadding(value, RIGHT);
            else if (key == "padding-top") c->SetPadding(value, TOP);
            else if (key == "padding") c->SetPadding(value);
            else if (key == "right") c->SetTether(RIGHT, value);
            else if (key == "top") c->SetTether(TOP, value);
            else if (key == "type") c->SetElementType(value);
            else if (key == "width") c->SetWidth(value);
            else
            {
                MX_THROW("Unknown property name"); // key appended below
            }
        }
        catch (std::exception const & err)
        {
            col = saveCol;
            Error(std::format("Error processing property \"{}\"=\"{}\" - {}", key, value, err.what()));
        }
     */
}