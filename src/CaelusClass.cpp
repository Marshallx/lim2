#include <algorithm>
#include <regex>

#include "CaelusClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Caelus
{
    CaelusClass * CaelusClassMap::GetClass(std::string_view const & name) const
    {
        for (auto c : m_map)
        {
            if (c.first == name) return c.second.get();
        }
        return nullptr;
    }

    void CaelusClassMap::GetClassChain(std::string_view const & name, std::vector<CaelusClass *> & chain) const
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

    Measure const * CaelusClassMap::GetPadding(Edge const edge, std::string_view const & name) const
    {
        auto chain = std::vector<CaelusClass *>{};
        GetClassChain(name, chain);
        for (auto const c : chain)
        {
            auto const p = c->GetPadding(edge);
            if (p) return p;
        }
        return nullptr;
    }

    Tether const * CaelusClassMap::GetTether(Edge const edge, std::string_view const & name) const
    {
        auto chain = std::vector<CaelusClass *>{};
        GetClassChain(name, chain);
        for (auto const c : chain)
        {
            auto const t = c->GetTether(edge);
            if (t) return t;
        }
        return nullptr;
    }

    void CaelusClass::AddClassNames(std::string_view const & classes)
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

    std::vector<std::string> const & CaelusClass::GetClassNames() const
    {
        return m_classNames;
    }

    CaelusElement const * CaelusClass::GetElement() const noexcept
    {
        return m_element;
    }

    std::string const & CaelusClass::GetName() const noexcept
    {
        return m_name;
    }

    std::string const & CaelusClass::GetParentName() const noexcept
    {
        return m_parentName;
    }

    Measure const * CaelusClass::GetPadding(Edge const edge) const
    {
        if (edge > 4) MX_THROW("Invalid padding edge");
        return m_padding[edge].has_value() ? &m_padding[edge].value() : nullptr;
    }

    Tether const * CaelusClass::GetTether(Edge const edge) const
    {
        if (edge > 4) MX_THROW("Invalid tether edge");
        return m_tethers[edge].has_value() ? &m_tethers[edge].value() : nullptr;
    }

    void CaelusClass::SetBackgroundColor(Color const & color)
    {
        m_backgroundColor = color;
    }

    void CaelusClass::SetBackgroundColor(uint32_t const color)
    {
        m_backgroundColor = { color };
    }

    void CaelusClass::SetBackgroundColor(std::string_view const & color)
    {
        SetBackgroundColor(Color::Parse(color));
    }

    void CaelusClass::SetContentAlignmentH(Edge const edge)
    {
        m_alignContentH = edge;
    }

    void CaelusClass::SetContentAlignmentV(Edge const edge)
    {
        m_alignContentV = edge;
    }

    void CaelusClass::SetElement(CaelusElement const * element)
    {
        m_element = element;
    }

    void CaelusClass::SetElementType(std::string_view const & type)
    {
        if (type == "button") { m_elementType = BUTTON; return; }
        if (type == "combobox") { m_elementType = COMBOBOX; return; }
        if (type == "edit") { m_elementType = EDITBOX; return; }
        if (type == "listbox") { m_elementType = LISTBOX; return; }
        if (type == "checkbox") { m_elementType = CHECKBOX; return; }
        if (type == "class") { m_elementType = CLASS; return; }
        m_elementType = GENERIC;
    }

    void CaelusClass::SetFontColor(Color const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void CaelusClass::SetFontColor(std::string_view const & color)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetColor(color);
    }

    void CaelusClass::SetFontFace(std::string_view const & face)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetFace(face);
    }

    void CaelusClass::SetFontSize(std::string_view const & size)
    {
        SetFontSize(size);
    }

    void CaelusClass::SetFontStyle(std::string_view const & style)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetStyle(style);
    }

    void CaelusClass::SetFontWeight(std::string_view const & weight)
    {
        if (!m_font.has_value()) m_font = Font{};
        m_font.value().SetWeight(weight);
    }

    void CaelusClass::SetHeight(std::string_view const & height)
    {
        m_size[HEIGHT] = Measure::Parse(height);
    }

    void CaelusClass::SetImagePath(std::filesystem::path const & path)
    {
        m_imagePath = path;
    }

    void CaelusClass::SetLabel(std::string_view const & v)
    {
        m_label = v;
    }

    void CaelusClass::SetOpacity(uint8_t const v)
    {
        m_opacity = v;
    }

    void CaelusClass::SetPadding(std::string_view const & padding, Edge const edge)
    {
        auto const m = Measure::Parse(padding);
        if (edge == Edge::ALL)
        {
            static auto const edges = { TOP, LEFT, BOTTOM, RIGHT };
            for (auto const edge : edges)
            {
                if (!m_padding[edge].has_value()) m_padding[edge] = m;
            }
        }
        else m_padding[edge] = m;
    }

    void CaelusClass::SetBorder(std::string const & border, Edge const edge)
    {
        auto toks = mxi::explode(border, " ");
        bool tokSolid = false;
        Measure tokMeasure = {};
        Color tokColor = {};

        for (size_t i = 0; i < toks.size(); ++i)
        {
            auto tok = toks[i];
            if (tok.empty()) continue;
            if (tok == "rgb(")
            {
                while (++i < toks.size() && toks[i][tok.size() - 1] != ')')
                {
                    tok.append(toks[i]);
                }
                tokColor = Color::Parse(tok);
                continue;
            }

            if (tok == "solid")
            {
                tokSolid = true;
                continue;
            }

            try
            {
                tokMeasure = Measure::Parse(tok);
            }
            catch (...) {};

            try
            {
                tokColor = Color::Parse(tok);
            }
            catch (...) {};

            MX_THROW(std::format("Unidentified border format token: {}", tok));
        }

        if (edge == Edge::ALL)
        {
            static auto const edges = { TOP, LEFT, BOTTOM, RIGHT };
            for (auto const edge : edges)
            {
                if (!m_borderWidth[edge].has_value()) m_borderWidth[edge] = tokMeasure;
                if (!m_borderColor[edge].has_value()) m_borderColor[edge] = tokColor;
            }
        }
        else m_borderWidth[edge] = tokMeasure;

    }

    void CaelusClass::SetBorderWidth(std::string_view const & width, Edge const edge)
    {
        auto const m = Measure::Parse(width);
        if (edge == Edge::ALL)
        {
            static auto const edges = { TOP, LEFT, BOTTOM, RIGHT };
            for (auto const edge : edges)
            {
                if (!m_borderWidth[edge].has_value()) m_borderWidth[edge] = m;
            }
        }
        else m_borderWidth[edge] = m;
    }

    void CaelusClass::SetBorderRadius(std::string_view const & radius, Corner const corner)
    {
        auto const m = Measure::Parse(radius);
        if (corner == Corner::ALL)
        {
            static auto const corners = { TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT };
            for (auto const corner : corners)
            {
                if (!m_borderRadius[corner].has_value()) m_borderRadius[corner] = m;
            }
        }
        else m_borderRadius[corner] = m;
    }

    void CaelusClass::SetParentName(std::string_view const & v)
    {
        m_parentName = v.empty() ? std::string{"window"} : std::string{v};
        if (m_name.starts_with('.')) MX_THROW("Classes cannot have a parent");
    }

    void CaelusClass::SetTether(Edge const myEdge, std::string_view const & otherId, Edge const otherEdge, Measure const & offset)
    {
        if (m_name == "window") return; // window has no parent or siblings so cannot be tethered
        m_tethers[myEdge] = { otherId, otherEdge, offset };
        // TODO Validate that no tether has cyclic dependencies
    }

    void CaelusClass::SetTether(Edge const myEdge, std::string_view const & tether)
    {
        /* Examples
            left=id.right+5px //explicit sibling
            left=5px //parent
            left=+5px // sibling
        */
        auto spec = std::string{ tether };
        mxi::trim(spec);
        constexpr static auto const pattern = R"((^(?:([^\.]+)\.(left|right|bottom|top|l|r|t|b))?(?:\s+)?(?:(\+|\-)?(?:\s+)([0-9]+(?:\.[0-9]+)?)(em|px|%|))?$))";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);

        static auto const msgFormat = "Invalid tether: bad format. Expected [id.side][[+|-]offset[px|em|%]], saw {}";
        static auto const msgEdge = "Invalid tether : incompatible edge axis.";

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW(std::format(msgFormat, spec).c_str());

        auto name = matches[1].str();
        auto const sedge = matches[2].str();
        auto const sop = matches[3].str();
        auto const soffset = matches[4].str();
        auto const sunit = matches[5].str();

        double offset = soffset.empty() ? 0 : atof(soffset.c_str());
        Edge otherEdge = myEdge;

        if (!name.empty())
        {
            // Normal tether
            otherEdge = edgeFromKeyword(sedge);
            if (isHEdge(otherEdge) != isHEdge(otherEdge)) MX_THROW(msgEdge);
            if (sop.empty() && !soffset.empty())  MX_THROW(std::format(msgFormat, spec).c_str());
            if (sop == "-") offset = -offset;
        }
        else if (sop.empty())
        {
            // Absolute within parent
            if (myEdge == BOTTOM || myEdge == RIGHT) offset = -offset;
        }
        else
        {
            // Tether to adjacent sibling
            name = ".";
            if (myEdge == TOP || myEdge == LEFT) offset = -offset;
            otherEdge = ~myEdge;
        }

        auto unit = Unit::PX;
        if (sunit == "em") unit = EM;
        else if (sunit == "%") { unit = PC; offset /= 100; }
        SetTether(myEdge, name, otherEdge, { offset, unit });
    }

    void CaelusClass::SetValue(std::string_view const & v)
    {
        m_value = v;
    }

    void CaelusClass::SetVisible(bool const v)
    {
        m_visible = v;
    }
    void CaelusClass::hide() { SetVisible(false); }
    void CaelusClass::show() { SetVisible(true); }

    void CaelusClass::SetWidth(std::string_view const & width)
    {
        if (width == "auto") m_size[WIDTH].reset();
        else m_size[WIDTH] = Measure::Parse(width);
    }
}