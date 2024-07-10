#include <format>

#include "MxiLogging.h"
#include "MxiUtils.h"

#include "jaml.h"
#include "CaelusElement.h"

namespace Caelus
{
    namespace
    {
        bool IsVoidElement(std::string_view const & tagname)
        {
            static constexpr auto const names = {
                "area", "base", "br", "col", "embed", "hr", "img", "input",
                "link", "meta", "param", "source", "track", "wbr"
            };
            for (auto const & name : names)
            {
                if (tagname == name) return true;
            }
            return false;
        }
    }

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

    [[noreturn]] void JamlParser::Error(std::string_view const & msg) const
    {
        MX_THROW(std::format("JAML parse error at {},{}: {}", line, col, msg));
    }

    void JamlParser::Expect(char const expected) const
    {
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

    JamlParser::JamlParser(std::string_view const & source, CaelusWindow & window) : source(source), e(window)
    {
        size = source.size();
        if (!size) Error("Empty document");
        c = source[0];
        EatCommentsAndWhitespace();
        ParseTag();
        if (e.m_tagname == "!doctype")
        {
            if (e.m_attributes.size() != 1 ||
                !e.m_attributes.contains("jaml") ||
                e.m_attributes["jaml"] != "")
            {
                Error("Unsupported doctype");
            }
            EatCommentsAndWhitespace();
            ParseTag();
            if (e.m_tagname != "jaml") Error("Outermost element should be \"jaml\"");
        }
        EatCommentsAndWhitespace();
        Expect(0);
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

    void JamlParser::ParseAttributes()
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

            e.m_attributes[unescape(ParseKey())] = unescape(ParseValue());
        }
    }

    void JamlParser::ParseTag()
    {
        Expect('<');
        NextChar();
        EatWhitespace();
        e.m_tagname = unescape(ParseName());
        if (e.m_tagname.empty()) Error("Expected an element name.");
        ParseAttributes();
        if (c == '/')
        {
            NextChar();
            Expect('>');
            return;
        }
        Eat(">");
        if (IsVoidElement(e.m_tagname)) return;
        ParseContent();
        Eat(std::format("/{}>", e.m_tagname));
    }

    void JamlParser::ParseContent()
    {
        RememberPos(true);
        for(auto start = 0; ; NextChar())
        {
            if (c == '<')
            {
                if (start)
                {
                    auto text = CaelusElement{};
                    text.m_text = unescape(mxi::trim(source.substr(start, pos - start)));
                    e.m_children.push_back(text);
                }
                if (LookAhead(std::format("/{}>", e.m_tagname), true)) return;
                if (LookAhead("/")) Error("Unexpected closing tag.");
                e = {};
                ParseTag();
                e.m_parent->m_children.push_back(e);
                e = *e.m_parent;
                continue;
            }
            if (!start) start = pos;
        }
        RememberPos(false);
        Error("Unterminated element.");
    }

    std::string unescape(std::string_view const & s)
    {
        static auto const escs = std::unordered_map<char const *, wchar_t const>
        {
            {"amp", L'&'}, {"lt", L'<'}, {"gt", L'>'}, {"quot", L'"'}, {"apos", L'\''}, {"Tab", L'\t'}, {"NewLine", L'\n'}, {"tab", L'\t'}, {"newline", L'\n'}, {"nbsp", 32},
            {"iexcl", 161}, {"cent", 162}, {"pound", 163}, {"curren", 164}, {"yen", 165}, {"brvbar", 166}, {"sect", 167}, {"uml", 168}, {"copy", 169}, {"ordf", 170}, {"laquo", 171}, {"not", 172}, {"shy", 173}, {"reg", 174}, {"macr", 175}, {"deg", 176},
            {"plusmn", 177}, {"sup2", 178}, {"sup3", 179}, {"acute", 180}, {"micro", 181}, {"para", 182}, {"dot", 182}, {"cedil", 184}, {"sup1", 185}, {"ordm", 186}, {"raquo", 187}, {"frac14", 188}, {"frac12", 189}, {"frac34", 190}, {"iquest", 191}, {"Agrave", 192},
            {"Aacute", 193}, {"Acirc", 194}, {"Atilde", 195}, {"Auml", 196}, {"Aring", 197}, {"AElig", 198}, {"Ccedil", 199}, {"Egrave", 200}, {"Eacute", 201}, {"Ecirc", 202}, {"Euml", 203}, {"Igrave", 204}, {"Iacute", 205}, {"Icirc", 206}, {"Iuml", 207}, {"ETH", 208},
            {"Ntilde", 209}, {"Ograve", 210}, {"Oacute", 211}, {"Ocirc", 212}, {"Otilde", 213}, {"Ouml", 214}, {"times", 215}, {"Oslash", 216}, {"Ugrave", 217}, {"Uacute", 218}, {"Ucirc", 219}, {"Uuml", 220}, {"Yacute", 221}, {"THORN", 222}, {"szlig", 223}, {"agrave", 224},
            {"aacute", 225}, {"acirc", 226}, {"atilde", 227}, {"auml", 228}, {"aring", 229}, {"aelig", 230}, {"ccedil", 231}, {"egrave", 232}, {"eacute", 233}, {"ecirc", 234}, {"euml", 235}, {"igrave", 236}, {"iacute", 237}, {"icirc", 238}, {"iuml", 239}, {"eth", 240},
            {"ntilde", 241}, {"ograve", 242}, {"oacute", 243}, {"ocirc", 244}, {"otilde", 245}, {"ouml", 246}, {"divide", 247}, {"oslash", 248}, {"ugrave", 249}, {"uacute", 250}, {"ucirc", 251}, {"uuml", 252}, {"yacute", 253}, {"thorn", 254}, {"yuml", 255}, {"Amacr", 256},
            {"amacr", 257}, {"Abreve", 258}, {"abreve", 259}, {"Aogon", 260}, {"aogon", 261}, {"Cacute", 262}, {"cacute", 263}, {"Ccirc", 264}, {"ccirc", 265}, {"Cdot", 266}, {"cdot", 267}, {"Ccaron", 268}, {"ccaron", 269}, {"Dcaron", 270}, {"dcaron", 271}, {"Dstrok", 272},
            {"dstrok", 273}, {"Emacr", 274}, {"emacr", 275}, {"Ebreve", 276}, {"ebreve", 277}, {"Edot", 278}, {"edot", 279}, {"Eogon", 280}, {"eogon", 281}, {"Ecaron", 282}, {"ecaron", 283}, {"Gcirc", 284}, {"gcirc", 285}, {"Gbreve", 286}, {"gbreve", 287}, {"Gdot", 288},
            {"gdot", 289}, {"Gcedil", 290}, {"gcedil", 291}, {"Hcirc", 292}, {"hcirc", 293}, {"Hstrok", 294}, {"hstrok", 295}, {"Itilde", 296}, {"itilde", 297}, {"Imacr", 298}, {"imacr", 299}, {"Ibreve", 300}, {"ibreve", 301}, {"Iogon", 302}, {"iogon", 303}, {"Idot", 304},
            {"inodot", 305}, {"IJlig", 306}, {"ijlig", 307}, {"Jcirc", 308}, {"jcirc", 309}, {"Kcedil", 310}, {"kcedil", 311}, {"kgreen", 312}, {"Lacute", 313}, {"lacute", 314}, {"Lcedil", 315}, {"lcedil", 316}, {"Lcaron", 317}, {"lcaron", 318}, {"Lmidot", 319}, {"lmidot", 320},
            {"Lstrok", 321}, {"lstrok", 322}, {"Nacute", 323}, {"nacute", 324}, {"Ncedil", 325}, {"ncedil", 326}, {"Ncaron", 327}, {"ncaron", 328}, {"napos", 329}, {"ENG", 330}, {"eng", 331}, {"Omacr", 332}, {"omacr", 333}, {"Obreve", 334}, {"obreve", 335}, {"Odblac", 336},
            {"odblac", 337}, {"OElig", 338}, {"oelig", 339}, {"Racute", 340}, {"racute", 341}, {"Rcedil", 342}, {"rcedil", 343}, {"Rcaron", 344}, {"rcaron", 345}, {"Sacute", 346}, {"sacute", 347}, {"Scirc", 348}, {"scirc", 349}, {"Scedil", 350}, {"scedil", 351}, {"Scaron", 352},
            {"scaron", 353}, {"Tcedil", 354}, {"tcedil", 355}, {"Tcaron", 356}, {"tcaron", 357}, {"Tstrok", 358}, {"tstrok", 359}, {"Utilde", 360}, {"utilde", 361}, {"Umacr", 362}, {"umacr", 363}, {"Ubreve", 364}, {"ubreve", 365}, {"Uring", 366}, {"uring", 367}, {"Udblac", 368},
            {"udblac", 369}, {"Uogon", 370}, {"uogon", 371}, {"Wcirc", 372}, {"wcirc", 373}, {"Ycirc", 374}, {"ycirc", 375}, {"Yuml", 376}, {"fnof", 402}, {"circ", 710}, {"tilde", 732}, {"Alpha", 913}, {"Beta", 914}, {"Gamma", 915}, {"Delta", 916}, {"Epsilon", 917},
            {"Zeta", 918}, {"Eta", 919}, {"Theta", 920}, {"Iota", 921}, {"Kappa", 922}, {"Lambda", 923}, {"Mu", 924}, {"Nu", 925}, {"Xi", 926}, {"Omicron", 927}, {"Pi", 928}, {"Rho", 929}, {"Sigma", 931}, {"Tau", 932}, {"Upsilon", 933}, {"Phi", 934},
            {"Chi", 935}, {"Psi", 936}, {"Omega", 937}, {"alpha", 945}, {"beta", 946}, {"gamma", 947}, {"delta", 948}, {"epsilon", 949}, {"zeta", 950}, {"eta", 951}, {"theta", 952}, {"iota", 953}, {"kappa", 954}, {"lambda", 955}, {"mu", 956}, {"nu", 957},
            {"xi", 958}, {"omicron", 959}, {"pi", 960}, {"rho", 961}, {"sigmaf", 962}, {"sigma", 963}, {"tau", 964}, {"upsilon", 965}, {"phi", 966}, {"chi", 967}, {"psi", 968}, {"omega", 969}, {"thetasym", 977}, {"upsih", 978}, {"piv", 982}, {"ensp", 8194},
            {"emsp", 8195}, {"thinsp", 8201}, {"zwnj", 8204}, {"zwj", 8205}, {"lrm", 8206}, {"rlm", 8207}, {"ndash", 8211}, {"mdash", 8212}, {"lsquo", 8216}, {"rsquo", 8217}, {"sbquo", 8218}, {"ldquo", 8220}, {"rdquo", 8221}, {"bdquo", 8222}, {"dagger", 8224}, {"Dagger", 8225},
            {"bull", 8226}, {"hellip", 8230}, {"permil", 8240}, {"prime", 8242}, {"Prime", 8243}, {"lsaquo", 8249}, {"rsaquo", 8250}, {"oline", 8254}, {"euro", 8364}, {"trade", 8482}, {"larr", 8592}, {"uarr", 8593}, {"rarr", 8594}, {"darr", 8595}, {"harr", 8596}, {"crarr", 8629},
            {"forall", 8704}, {"part", 8706}, {"exist", 8707}, {"empty", 8709}, {"nabla", 8711}, {"isin", 8712}, {"notin", 8713}, {"ni", 8715}, {"prod", 8719}, {"sum", 8721}, {"minus", 8722}, {"lowast", 8727}, {"radic", 8730}, {"prop", 8733}, {"infin", 8734}, {"ang", 8736},
            {"and", 8743}, {"or", 8744}, {"cap", 8745}, {"cup", 8746}, {"int", 8747}, {"there4", 8756}, {"sim", 8764}, {"cong", 8773}, {"asymp", 8776}, {"ne", 8800}, {"equiv", 8801}, {"le", 8804}, {"ge", 8805}, {"sub", 8834}, {"sup", 8835}, {"nsub", 8836},
            {"sube", 8838}, {"supe", 8839}, {"oplus", 8853}, {"otimes", 8855}, {"perp", 8869}, {"sdot", 8901}, {"lceil", 8968}, {"rceil", 8969}, {"lfloor", 8970}, {"rfloor", 8971}, {"loz", 9674}, {"spades", 9824}, {"clubs", 9827}, {"hearts", 9829}, {"diams", 9830}
        };
        auto oss = std::ostringstream{};
        size_t pos = 0;
        size_t amp = 0;
        while ((amp = s.find_first_of('&', pos)) != std::string::npos)
        {
            oss << s.substr(pos, amp - pos);
            ++amp;
            auto const sem = s.find_first_of(';', amp);
            if (sem - amp > 10)
            {
                MX_LOG_WARN(std::format("Unterminated HTML escape sequence: {}...", s.substr(0, 8)));
                oss << s[pos];
                ++pos;
                continue;
            }
            auto const esc = s.substr(amp, sem - amp);
            wchar_t w = 0;
            if (esc.size() && esc[0] == '#')
            {
                // TODO validate digits
                if (esc.size() >= 2 && (esc[1] == 'x' || esc[1] == 'X'))
                {
                    unsigned int x;
                    std::stringstream ss;
                    ss << std::hex << esc.substr(2);
                    ss >> x;
                    w = x;
                }
                else
                {
                    auto const sub = std::string{esc.substr(1)};
                    w = atoi(sub.c_str());
                }
            }
            else
            {
                auto const sub = std::string{ esc.substr(1) };
                if (escs.contains(sub.c_str())) w = escs.at(sub.c_str());
            }
            if (!w)
            {
                MX_LOG_WARN(std::format("Unrecognized HTML escape sequence: {}", esc));
                oss << s[pos];
                ++pos;
                continue;
            }
            oss << mxi::Utf8String(std::wstring{w});
            pos = sem + 1;
        }
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