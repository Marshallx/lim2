#include <format>

#include "MxiLogging.h"
#include "MxiUtils.h"

#include "jaml.h"

namespace jaml
{
    class JamlParser
    {
    public:
        JamlParser(std::string_view const & source, Doc & doc);
        std::string_view const & source;
        char c = 0;
        size_t size;
        size_t pos = 0;
        size_t col = 0;
        size_t line = 0;

        void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Expected(std::string_view const & expected) const;
        void Expected(char const expected) const;
        void EoiCheck(std::string_view const & expected) const;
        void EoiCheck(char const expected) const;
        void Eat(std::string_view const & expected);
        void RememberPos(bool const stash = true);
        bool LookAhead(std::string_view const & expected, bool const eatIfFound = false);
        void EatWhitespace();
        void EatCommentsAndWhitespace();
        char NextChar();
        std::string_view ParseName();
        std::string_view ParseKey();
        std::string_view ParseValue();
        void ParseAttributes(Element & e);
        void ParseContent(Element & e);
        Element ParseTag();
    };

    void JamlParser::RememberPos(bool const stash)
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

    void JamlParser::Error(std::string_view const & msg) const
    {
        MX_THROW(std::format("Caelus parse error at {},{}: {}", line, col, msg));
    }

    void JamlParser::Expect(char const expected) const
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
        if (c == expected) return;
        Error(std::format("Expected '{}'", expected));
    }

    void JamlParser::Eat(std::string_view const & expected)
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

    void JamlParser::EatWhitespace()
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

    bool JamlParser::LookAhead(std::string_view const & expected, bool eatIfFound)
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

    void JamlParser::EatCommentsAndWhitespace()
    {
        for(;;)
        {
            EatWhitespace();
            if (c != '<') return;
            if (!LookAhead("!--")) return;
            auto const sub = source.substr(pos + 3);
            auto n = sub.find("-->");
            if (n == std::string::npos) Error("Unterminated comment.");
            n += 3;
            while (pos != n) NextChar();
        }
    }

    char JamlParser::NextChar()
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

    JamlParser::JamlParser(std::string_view const & source, Doc & doc) : source(source)
    {
        size = source.size();
        if (!size) Error("Empty document");
        c = source[0];
        EatCommentsAndWhitespace();
        auto tag = ParseTag();
        if (tag.type == "!doctype")
        {
            if (tag.attributes.size() != 1 ||
                tag.attributes[0].first != "jaml" ||
                tag.attributes[0].second != "")
            {
                Error("Unsupported doctype");
            }
            EatCommentsAndWhitespace();
            tag = ParseTag();
        }
        EatCommentsAndWhitespace();
        Expect(0);
        doc.outer = tag;
    }

    std::string_view JamlParser::ParseName()
    {
        auto const start = pos;
        for (;; NextChar())
        {
            switch (c)
            {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '=':
            case '/':
            case '>':
                return { source.begin() + start, source.begin() + pos };
            }
        }
    }

    std::string_view JamlParser::ParseKey()
    {
        return ParseName();
    }

    std::string_view JamlParser::ParseValue()
    {
        EatWhitespace();
        if (c != '=') return {};
        NextChar();
        EatWhitespace();
        auto quoted = c == '"';
        if (quoted) NextChar();
        auto const start = pos;
        for (;; NextChar())
        {
            if (!quoted)
            {
                switch (c)
                {
                case 0:
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                case '/':
                case '>':
                    break;
                default:
                    continue;
                }
                break;
            }

            switch (c)
            {
            case 0:
            case '\r':
            case '\n':
                Error("Unterminated string.");
            case '"':
                break;
            }
            break;
        }
        return { source.begin() + start, source.begin() + pos };
    }

    void JamlParser::ParseAttributes(Element & e)
    {
        while (c)
        {
            EatWhitespace();
            switch (c)
            {
            case 0:
            case '/':
            case '>':
                return;
            }

            e.attributes.push_back({
                std::string{ ParseKey() },
                std::string{ ParseValue() }
            });
        }
    }

    Element JamlParser::ParseTag()
    {
        Expect('<');
        NextChar();
        EatWhitespace();
        Element e = {};
        e.type = ParseName();
        if (e.type.empty()) Error("Expected an element name.");
        ParseAttributes(e);
        if (c == '/')
        {
            NextChar();
            Expect('>');
            return e;
        }
        Expect('>');
        ParseContent(e);
        Eat(std::format("/{}>", e.type));
        return e;
    }

    void JamlParser::ParseContent(Element & e)
    {
        auto start = 0;
        RememberPos(true);
        while (NextChar())
        {
            if (c == '<')
            {
                if (start)
                {
                    auto text = Element{};
                    text.text = source.substr(start, pos - start);
                    e.children.push_back(text);
                }
                if (LookAhead(std::format("/{}>", e.type), true)) return;
                if (LookAhead("/")) Error("Unexpected closing tag.");
                e.children.push_back(ParseTag());
                continue;
            }
            if (!start) start = pos;
        }
        RememberPos(false);
        Error("Unterminated element.");
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