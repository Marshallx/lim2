#include <algorithm>
#include <regex>

#include "JamlClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace jaml
{
    JamlClass * JamlClassMap::GetClass(std::string_view const & name) const
    {
        for (auto c : m_map)
        {
            if (c.first == name) return c.second.get();
        }
        return nullptr;
    }

    void JamlClassMap::GetClassChain(std::string_view const & name, std::vector<JamlClass *> & chain) const
    {
        auto const c = GetClass(name);
        if (!c) return;
        if (std::find(chain.begin(), chain.end(), c) != chain.end()) return;
        chain.push_back(c);
        for (auto const & name2 : c->GetClassNames())
        {
            GetClassChain(name2, chain);
        }
    }

    Tether const * JamlClassMap::GetTether(Edge const edge, std::string_view const & name) const
    {
        auto chain = std::vector<JamlClass *>{};
        GetClassChain(name, chain);
        for (auto const c : chain)
        {
            auto const t = c->GetTether(edge);
            if (t) return t;
        }
        return nullptr;
    }

    void JamlClass::AddClassNames(std::string_view const & classes)
    {
        auto more = mxi::explode(classes);
        // Add classes in reverse order (priority order in source is RTL)
        for (auto i = more.size() - 1; i>=0; --i)
        {
            mxi::trim(more[i]);
            if (std::find(m_classNames.begin(), m_classNames.end(), more[i]) != m_classNames.end()) continue;
            m_classNames.push_back(std::string{ more[i]} );
        }
    }

    std::vector<std::string> const & JamlClass::GetClassNames() const
    {
        return m_classNames;
    }

    JamlElement const * JamlClass::GetElement() const noexcept
    {
        return m_element;
    }

    std::string const & JamlClass::GetName() const noexcept
    {
        return m_name;
    }

    std::string const & JamlClass::GetParentName() const noexcept
    {
        return m_parentName;
    }

    Tether const * JamlClass::GetTether(Edge const edge) const
    {
        if (edge > 4) MX_THROW("Invalid tether edge");
        return m_tethers[edge].has_value() ? &m_tethers[edge].value() : nullptr;
    }

    void JamlClass::SetBackgroundColor(Color const & color)
    {
        m_backgroundColor = color;
    }

    void JamlClass::SetBackgroundColor(uint32_t const color)
    {
        m_backgroundColor = { color };
    }

    void JamlClass::SetBackgroundColor(std::string_view const & color)
    {
        SetBackgroundColor(Color::Parse(color));
    }

    void JamlClass::SetContentAlignmentH(Edge const edge)
    {
        m_alignContentH = edge;
    }

    void JamlClass::SetContentAlignmentV(Edge const edge)
    {
        m_alignContentV = edge;
    }

    void JamlClass::SetElement(JamlElement const * element)
    {
        m_element = element;
    }

    void JamlClass::SetElementType(JamlElementType const type)
    {
        m_elementType = type;
    }

    void JamlClass::SetElementType(std::string_view const & type)
    {
        if (type == "button") { m_elementType = BUTTON; return; }
        if (type == "combobox") { m_elementType = COMBOBOX; return; }
        if (type == "edit") { m_elementType = EDITBOX; return; }
        if (type == "listbox") { m_elementType = LISTBOX; return; }
        if (type == "checkbox") { m_elementType = CHECKBOX; return; }
        if (type == "class") { m_elementType = CLASS; return; }
        m_elementType = GENERIC;
    }

    void JamlClass::SetFontColor(Color const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void JamlClass::SetFontColor(std::string_view const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void JamlClass::SetFontFace(std::string_view const & face)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetFace(face);
    }

    void JamlClass::SetFontSize(std::string_view const & size)
    {
        SetFontSize(size);
    }

    void JamlClass::SetFontStyle(std::string_view const & style)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetStyle(style);
    }

    void JamlClass::SetFontWeight(std::string_view const & weight)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetWeight(weight);
    }

    void JamlClass::SetFontWeight(int const weight)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetWeight(weight);
    }

    void JamlClass::SetHeight(std::string_view const & height)
    {
        m_size[HEIGHT] = Measure::Parse(height);
    }

    void JamlClass::SetImagePath(std::filesystem::path const & path)
    {
        m_imagePath = path;
    }

    void JamlClass::SetLabel(std::string_view const & v)
    {
        m_label = v;
    }

    void JamlClass::SetOpacity(uint8_t const v)
    {
        m_opacity = v;
    }

    void JamlClass::SetPadding(Edge const edge, Measure const & v)
    {
        m_padding[edge] = v;
    }

    void JamlClass::SetParentName(std::string_view const & v)
    {
        m_parentName = v.empty() ? std::string{"window"} : std::string{v};
        if (m_name.starts_with('.')) MX_THROW("Classes cannot have a parent");
    }

    void JamlClass::SetTether(Edge const mySide, std::string_view const & otherId, Edge const otherSide, Measure const & offset)
    {
        if (m_name == "window") return; // window has no parent or siblings so cannot be tethered
        m_tethers[mySide] = { otherId, otherSide, offset };
        // TODO Validate that no tether has cyclic dependencies
    }

    void JamlClass::SetTether(Edge const mySide, std::string const & spec)
    {
        constexpr static auto const pattern = R"(^([^>]+)>(left|right|bottom|top|l|r|t|b)(?:(\+|\-[0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?)?$)";
        constexpr static auto const simplePattern = R"(^(\-?[0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);
        static auto const simpleRegex = std::regex(simplePattern, std::regex_constants::ECMAScript);

        auto matches = std::smatch{};
        if (std::regex_search(spec, matches, regex))
        {
            auto otherSide = mySide;
            switch (matches[2].str()[0])
            {
            case 't': otherSide = TOP; break;
            case 'l': otherSide = LEFT; break;
            case 'b': otherSide = BOTTOM; break;
            case 'r': otherSide = RIGHT; break;
            }
            if (isHEdge(mySide) != isHEdge(otherSide)) MX_THROW("Invalid tether: incompatible edge axis.");

            double offset = (matches.length() >= 3) ? atof(matches[3].str().c_str()) : 0;
            std::string const unitStr = (matches.length() >= 4) ? matches[4].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") { unit = PC; offset /= 100; }

            SetTether(mySide, matches[1].str(), otherSide, { offset, unit });
        }
        else if ( std::regex_search(spec, matches, simpleRegex))
        {
            double offset = atof(matches[1].str().c_str());
            if (mySide == BOTTOM || mySide == RIGHT) offset = -offset;
            std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") MX_THROW("Invalid tether: % not implemented.");
            SetTether(mySide, {}, mySide, {offset, unit});
        }
        else MX_THROW(std::format("Invalid tether: bad format. Expected [id>side][±offset em|px], saw {}", spec).c_str());
    }

    void JamlClass::SetValue(std::string_view const & v)
    {
        m_value = v;
    }

    void JamlClass::SetVisible(bool const v)
    {
        m_visible = v;
    }
    void JamlClass::hide() { SetVisible(false); }
    void JamlClass::show() { SetVisible(true); }

    void JamlClass::SetWidth(std::string_view const & width)
    {
        if (width == "auto") m_size[WIDTH].reset();
        else m_size[WIDTH] = Measure::Parse(width);
    }
}