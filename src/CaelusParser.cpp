#include "CaelusClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

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
        MX_THROW(std::format("Caelus parse error at {},{}: {}", lineno, col, msg));
    }

    void CaelusParser::Expect(char const expected) const
    {
        switch (expected)
        {
        case 0:
        case '\r':
        case '\n':
        case ';':
            break;
        default:
            EoiCheck(expected);
        }
        if (Peek() == expected) return;
        Expected(expected);
    }

    void CaelusParser::Expected(std::string_view const & expected) const
    {
        Error(std::format("Expected {}", expected));
    }

    void CaelusParser::Expected(char const expected) const
    {
        if (expected) Error(std::format("Expected '{}'", expected));
        Error("Expected end of line (or ';')");
    }

    /*
    void CaelusParser::EoiCheck(std::string_view const & expected) const
    {
        if (loc.pos < source.size()) return;
        Error(std::format("Unexpected end of input. Expected {}", expected));
    }
    */

    void CaelusParser::EoiCheck(char const expected) const
    {
        switch (Peek())
        {
        case 0:
        case ';':
        case '\r':
        case '\n':
            Error(std::format("Unexpected end of input. Expected '{}'", expected));
        }
    }

    char CaelusParser::Peek() const
    {
        if (col >= linetext.size()) return 0;
        return linetext[col];
    }

    bool CaelusParser::IsWhitespace() const
    {
        return isWhitespace(Peek());
    }

    void CaelusParser::EatWhitespace()
    {
        for(;;)
        {
            switch (Peek())
            {
            case ';':
                col = linetext.size();
                return;
            case 0:
            case '\n':
                ++col;
                return;
            case ' ':
            case '\t':
            case '\r':
                ++col;
                continue;
            default:
                return;
            }
        }
    }

    void CaelusParser::NextLine()
    {
        linetext = lines[lineno];
        ++lineno;
        col = 0;
    }

    CaelusParser::CaelusParser(std::string_view const & source, CaelusClassMap & classes)
    {
        lines = mxi::explode(source, "\n");
        NextLine();
        while(lineno<lines.size())
        {
            EatWhitespace();
            switch (Peek())
            {
            case 0:
                continue;
            case '[':
                // New section
                ParseSection(classes);
                continue;
            default:
                Error("Unknown");
            }
        }
    };

    void CaelusParser::ParseSection(CaelusClassMap & classes)
    {
        Expect('[');
        ++col;

        // Read section name
        auto name = std::string{};
        auto start = col;
        EatWhitespace();
        for(;; ++col)
        {
            EoiCheck(']');
            if (IsWhitespace())
            {
                EatWhitespace();
                Expect(']');
            }
            if (Peek() != ']') continue;
            name = linetext.substr(start, col - 1);
            ++col;
            break;
        }
        EatWhitespace();
        Expect(0);
        auto cp = classes.m_map.contains(name.c_str()) ? classes.m_map[name] : std::make_shared<CaelusClass>(CaelusClass{name, &classes});
        classes.m_map[name] = cp;
        auto c = cp.get();
        if (!name.starts_with('.')) c->SetParentName("window");

        while (lineno < lines.size())
        {
            NextLine();
            EatWhitespace();
            switch (Peek())
            {
            case 0: continue;
            case '[': return;
            }
            auto saveCol = col;
            auto const key = mxi::trim(ParseKey());
            Expect('=');
            ++col;
            EatWhitespace();
            saveCol = col;
            auto const value = mxi::trim(ParseValue());
            EatWhitespace();
            Expect(0);
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
                else if (key == "input-type") c->SetInputType(value);
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
        }
    }

    std::string_view CaelusParser::ParseKey()
    {
        auto const start = col;
        for (;; ++col)
        {
            switch (Peek())
            {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case ';':
                Expect('=');
            case '=':
                return { linetext.begin() + start, linetext.begin() + col };
            }
        }
    }

    std::string_view CaelusParser::ParseValue()
    {
        auto const start = col;
        for (;; ++col)
        {
            switch (Peek())
            {
            case 0:
            case '\r':
            case '\n':
            case ';':
                return { linetext.begin() + start, linetext.begin() + col };
            }
        }
    }

}