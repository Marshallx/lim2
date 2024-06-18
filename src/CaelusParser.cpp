#include "CaelusClass.h"
#include "MxiLogging.h"

#include "CaelusParser.h"

namespace Caelus
{
    namespace
    {
        bool isWhitespace(char const c)
        {
            switch (c)
            {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                return true;
            }
            return false;
        }
    }

    void CaelusParser::Error(std::string_view const & msg) const
    {
        MX_THROW(std::format("Caelus parse error at {},{}: {}", loc.line, loc.col, msg));
    }

    void CaelusParser::Expect(char const expected) const
    {
        EoiCheck(expected);
        if (source[loc.pos] == expected) return;
        Expected(expected);
    }

    void CaelusParser::Expected(std::string_view const & expected) const
    {
        Error(std::format("Expected {}", expected));
    }

    void CaelusParser::Expected(char const expected) const
    {
        Error(std::format("Expected '{}'", expected));
    }

    void CaelusParser::EoiCheck(std::string_view const & expected) const
    {
        if (loc.pos < source.size()) return;
        Error(std::format("Unexpected end of input. Expected {}", expected));
    }

    void CaelusParser::EoiCheck(char const expected) const
    {
        if (loc.pos < source.size()) return;
        Error(std::format("Unexpected end of input. Expected '{}'", expected));
    }

    void CaelusParser::NextChar()
    {
        ++loc.pos;
        ++loc.col;
    }

    char CaelusParser::Peek() const
    {
        if (loc.pos >= source.size()) return 0;
        return source[loc.pos];
    }

    bool CaelusParser::IsWhitespace() const
    {
        return isWhitespace(source[loc.pos]);
    }

    void CaelusParser::EatWhitespace(bool const eatLF)
    {
        while (loc.pos < source.size())
        {
            switch (source[loc.pos])
            {
            case '\n':
                if (!eatLF) return;
                ++loc.pos;
                ++loc.line;
                loc.col = 1;
                continue;
            case ' ':
            case '\t':
            case '\r':
                NextChar();
                continue;
            }
            break;
        }
    }

    void CaelusParser::EatComments()
    {
        while (loc.pos < source.size())
        {
            EatWhitespace();
            if (loc.pos >= source.size()) return;
            if (source[loc.pos] == ';')
            {
                while (loc.pos < source.size())
                {
                    if (source[loc.pos] == '\n')
                    {
                        ++loc.pos;
                        ++loc.line;
                        loc.col = 1;
                        break;
                    }
                    NextChar();
                }
                continue;
            }
            NextChar();
            return;
        }
    }

    CaelusParser::CaelusParser(std::string_view const & source, CaelusClassMap & classes) : source(source)
    {
        while (loc.pos < source.size())
        {
            EatComments();
            if (loc.pos >= source.size()) return;
            ParseSection(classes);
        }
    };

    void CaelusParser::ParseSection(CaelusClassMap & classes)
    {
        Expect('[');
        NextChar();
        EatWhitespace();

        // Read section name
        auto name = std::string{};
        auto start = loc.pos;
        for(;; NextChar())
        {
            EoiCheck(']');
            if (IsWhitespace())
            {
                EatWhitespace(false);
                Expect(']');
            }
            if (source[loc.pos] == ']')
            {
                name = source.substr(start, loc.pos);
                if (classes.contains(name.c_str()))
                {
                    Error(std::format("Duplicate {} name \"{}\"", name.starts_with('.') ? "class" : "element", name));
                }
                NextChar();
                break;
            }
            if (source[loc.pos] == ';') Expected(']');
        }
        auto cp = std::make_shared<CaelusClass>(CaelusClass{name});
        classes[name] = cp;
        auto c = cp.get();
        for (;; NextChar())
        {
            EatComments();
            if (loc.pos >= source.size()) return;
            if (source[loc.pos] == '[') return;
            auto keyStart = loc;
            auto const key = ParseKey();
            EatComments();
            Expect('=');
            NextChar();
            EatComments();
            auto valStart = loc;
            auto const value = ParseValue();
            try
            {
                if (key == "parent") c->SetParentName(value);
                else if (key == "background-color" || key == "bgcolor") c->SetBackgroundColor(value);
                else if (key == "bottom") c->SetTether(BOTTOM, value);
                else if (key == "class" || key == "classes") c->AddClassNames(value);
                else if (key == "font-color" || key == "fontcolor" || key == "color" || key == "fgcolor" || key == "text-color") c->SetFontColor(value);
                else if (key == "font-face" || key == "fontface") c->SetFontFace(value);
                else if (key == "font-size" || key == "fontsize") c->SetFontSize(value);
                else if (key == "height") c->SetHeight(value);
                else if (key == "label") c->SetLabel(value);
                else if (key == "left") c->SetTether(LEFT, value);
                else if (key == "max-width") c->SetMaxWidth(value);
                else if (key == "min-width") c->SetMinWidth(value);
                else if (key == "right") c->SetTether(RIGHT, value);
                else if (key == "top") c->SetTether(TOP, value);
                else if (key == "type" || key == "element-type") c->SetElementType(value);
                else if (key == "width") c->SetWidth(value);
                else
                {
                    MX_THROW("Unknown property name"); // key appended below
                }
            }
            catch (std::exception const & err)
            {
                loc = keyStart;
                Error(std::format("Error processing property \"{}\"=\"{}\" - {}", key, value, err.what()));
            }
        }
    }

    std::string_view CaelusParser::ParseKey()
    {
        auto const start = loc.pos;
        for (;; NextChar())
        {
            EoiCheck("a property name");
            if (source[loc.pos] != '-' && (source[loc.pos] < 'a' || source[loc.pos] > 'z'))
            {
                return { source.begin() + start, source.begin() + loc.pos };
            }
        }
    }

    std::string CaelusParser::ParseValue()
    {
        auto oss = std::ostringstream{};
        bool const quoted = source[loc.pos] == '"';
        if (quoted) NextChar();
        for (;; NextChar())
        {
            EoiCheck("a property value");
            switch (source[loc.pos])
            {
            case '\r':
            case '\n':
            case ' ':
            case '\t':
            case '}':
            case '=':
            case ';':
                if (!quoted)
                {
                    NextChar();
                    return oss.str();
                }
                if (source[loc.pos] == '\r' || source[loc.pos] == '\n') Error("Unterminated string");
                oss << source[loc.pos];
                continue;

            case '"':
                if (!quoted) Error("Unexpected '\"' in property value");
                NextChar();
                return oss.str();

            case ('\\'):
                NextChar();
                EoiCheck("an escape sequence");
                switch (source[loc.pos])
                {
                case '"':  oss << '"'; continue;
                case '\\': oss << '\\'; continue;
                case 'b': oss << '\b'; continue;
                case 'f': oss << '\f'; continue;
                case 'n': oss << '\n'; continue;
                case 'r': oss << '\r'; continue;
                case 't': oss << '\t'; continue;
                case 'u':
                    // TODO
                    [[fallthrough]];
                default:
                    Error(std::format("Unrecognized escape sequence \\{}", source[loc.pos]));
                }
                MX_THROW("Unreachable");

            default:
                oss << source[loc.pos];
                continue;
            }
            MX_THROW("Unreachable");
        }
    }
}