#include <algorithm>
#include <regex>

#include "AnusClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Anus
{
    AnusClass * AnusClassMap::GetClass(std::string_view const & name) const
    {
        for (auto c : m_map)
        {
            if (c.first == name) return c.second.get();
        }
        return nullptr;
    }

    void AnusClassMap::GetClassChain(std::string_view const & name, std::vector<AnusClass *> & chain) const
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

    Tether const * AnusClassMap::GetTether(Edge const edge, std::string_view const & name) const
    {
        auto chain = std::vector<AnusClass *>{};
        GetClassChain(name, chain);
        for (auto const c : chain)
        {
            auto const t = c->GetTether(edge);
            if (t) return t;
        }
        return nullptr;
    }

    void AnusClass::AddClassNames(std::string_view const & classes)
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

    std::vector<std::string> const & AnusClass::GetClassNames() const
    {
        return m_classNames;
    }

    AnusElement const * AnusClass::GetElement() const noexcept
    {
        return m_element;
    }

    std::string const & AnusClass::GetName() const noexcept
    {
        return m_name;
    }

    std::string const & AnusClass::GetParentName() const noexcept
    {
        return m_parentName;
    }

    Tether const * AnusClass::GetTether(Edge const edge) const
    {
        if (edge > 4) MX_THROW("Invalid tether edge");
        return m_tethers[edge].has_value() ? &m_tethers[edge].value() : nullptr;
    }

    void AnusClass::SetBackgroundColor(Color const & color)
    {
        m_backgroundColor = color;
    }

    void AnusClass::SetBackgroundColor(uint32_t const color)
    {
        m_backgroundColor = { color };
    }

    void AnusClass::SetBackgroundColor(std::string_view const & color)
    {
        SetBackgroundColor(Color::Parse(color));
    }

    void AnusClass::SetContentAlignmentH(Edge const edge)
    {
        m_alignContentH = edge;
    }

    void AnusClass::SetContentAlignmentV(Edge const edge)
    {
        m_alignContentV = edge;
    }

    void AnusClass::SetElement(AnusElement const * element)
    {
        m_element = element;
    }

    void AnusClass::SetElementType(AnusElementType const type)
    {
        m_elementType = type;
    }

    void AnusClass::SetElementType(std::string_view const & type)
    {
        if (type == "button") { m_elementType = BUTTON; return; }
        if (type == "combobox") { m_elementType = COMBOBOX; return; }
        if (type == "edit") { m_elementType = EDITBOX; return; }
        if (type == "listbox") { m_elementType = LISTBOX; return; }
        if (type == "checkbox") { m_elementType = CHECKBOX; return; }
        if (type == "class") { m_elementType = CLASS; return; }
        m_elementType = GENERIC;
    }

    void AnusClass::SetFontColor(Color const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void AnusClass::SetFontColor(std::string_view const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void AnusClass::SetFontFace(std::string_view const & face)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetFace(face);
    }

    void AnusClass::SetFontSize(std::string_view const & size)
    {
        SetFontSize(size);
    }

    void AnusClass::SetFontStyle(std::string_view const & style)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetStyle(style);
    }

    void AnusClass::SetFontWeight(std::string_view const & weight)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetWeight(weight);
    }

    void AnusClass::SetFontWeight(int const weight)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetWeight(weight);
    }

    void AnusClass::SetHeight(std::string_view const & height)
    {
        m_size[HEIGHT] = Measure::Parse(height);
    }

    void AnusClass::SetImagePath(std::filesystem::path const & path)
    {
        m_imagePath = path;
    }

    void AnusClass::SetLabel(std::string_view const & v)
    {
        m_label = v;
    }

    void AnusClass::SetOpacity(uint8_t const v)
    {
        m_opacity = v;
    }

    void AnusClass::SetPadding(Edge const edge, Measure const & v)
    {
        m_padding[edge] = v;
    }

    void AnusClass::SetParentName(std::string_view const & v)
    {
        m_parentName = v.empty() ? std::string{"window"} : std::string{v};
        if (m_name.starts_with('.')) MX_THROW("Classes cannot have a parent");
    }

    void AnusClass::SetTether(Edge const myEdge, std::string_view const & otherId, Edge const otherEdge, Measure const & offset)
    {
        if (m_name == "window") return; // window has no parent or siblings so cannot be tethered
        m_tethers[myEdge] = { otherId, otherEdge, offset };
        // TODO Validate that no tether has cyclic dependencies
    }

    void AnusClass::SetTether(Edge const myEdge, std::string const & spec)
    {
        /* Examples
            left=id>right+5px

        */
        constexpr static auto const pattern = R"(^(?:\s+)?([^>]+)(?:\s+)?>(?:\s+)?(left|right|bottom|top|l|r|t|b)(?:\s+)?(?:(\+|\-[0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?)?(?:\s+)?$)";
        constexpr static auto const adjacentPattern =                                                /**/R"(^(?:\s+)?>(?:\s+)?(?:([0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?)?(?:\s+)?$)";
        constexpr static auto const absolutePattern =                                                              /**/R"(^(?:\s+)?([0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?(?:\s+)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);
        static auto const adjacentRegex = std::regex(adjacentPattern, std::regex_constants::ECMAScript);
        static auto const absoluteRegex = std::regex(absolutePattern, std::regex_constants::ECMAScript);

        auto matches = std::smatch{};
        if (std::regex_search(spec, matches, regex))
        {
            // Normal tether
            auto otherEdge = edgeFromKeyword(matches[2].str());
            if (isHEdge(myEdge) != isHEdge(otherEdge)) MX_THROW("Invalid tether: incompatible edge axis.");
            double offset = (matches.length() >= 3) ? atof(matches[3].str().c_str()) : 0;
            std::string const unitStr = (matches.length() >= 4) ? matches[4].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") { unit = PC; offset /= 100; }

            SetTether(myEdge, matches[1].str(), otherEdge, { offset, unit });
        }
        else if (std::regex_search(spec, matches, absoluteRegex))
        {
            // Absolute within parent
            double offset = atof(matches[1].str().c_str());
            if (myEdge == BOTTOM || myEdge == RIGHT) offset = -offset;
            std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") MX_THROW("Invalid tether: % not implemented.");
            SetTether(myEdge, {}, myEdge, {offset, unit});
        }
        else if (std::regex_search(spec, matches, adjacentRegex))
        {
            // Tether to adjacent sibling
            double offset = atof(matches[1].str().c_str());
            if (myEdge == TOP || myEdge == LEFT) offset = -offset;
            std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") MX_THROW("Invalid tether: % not implemented.");
            SetTether(myEdge, ">", ~myEdge, {offset, unit});
        }
        else MX_THROW(std::format("Invalid tether: bad format. Expected [id>side][±offset em|px], saw {}", spec).c_str());
    }

    void AnusClass::SetValue(std::string_view const & v)
    {
        m_value = v;
    }

    void AnusClass::SetVisible(bool const v)
    {
        m_visible = v;
    }
    void AnusClass::hide() { SetVisible(false); }
    void AnusClass::show() { SetVisible(true); }

    void AnusClass::SetWidth(std::string_view const & width)
    {
        if (width == "auto") m_size[WIDTH].reset();
        else m_size[WIDTH] = Measure::Parse(width);
    }
}