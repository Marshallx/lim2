#include <regex>

#include "CaelusFont.h"
#include "MxiLogging.h"

#include "CaelusMeasure.h"


namespace Caelus
{

    /*
    int du2px(HWND const hwnd, int const du, Axis axis)
    {
        RECT r = {};
        r.left = r.top = du;
        r.right = r.bottom = 0;
        MapDialogRect(hwnd, &r);
        return axis == X ? r.left : r.top;
    }
    */

    std::string edgeToKeyword(Edge const edge)
    {
        switch (edge)
        {
        case TOP: return "top";
        case LEFT: return "left";
        case BOTTOM: return "bottom";
        case RIGHT: return "right";
        case CENTER: return "center";
        }
        MX_THROW("Unknown edge.");
    }

    Edge keywordToEdge(std::string_view const & keyword)
    {
        switch (keyword[0])
        {
        case 't': return TOP;
        case 'l': return LEFT;
        case 'b': return BOTTOM;
        case 'r': return RIGHT;
        }
        MX_THROW("Unknown edge.");
    }

    int getDpi(HWND hwnd)
    {
        HDC hdc = GetDC(hwnd);
        if (!hdc) return 0;
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(0, hdc);
        return dpi;
    }

    bool isHEdge(Edge const edge)
    {
        return edge == LEFT || edge == RIGHT;
    }

    bool isVEdge(Edge const edge)
    {
        return edge == TOP || edge == BOTTOM;
    }

    bool isFarEdge(Edge const edge)
    {
        return edge == RIGHT || edge == BOTTOM;
    }

    Edge operator ~(Edge const edge)
    {
        switch (edge)
        {
        case TOP: return BOTTOM;
        case BOTTOM: return TOP;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        }
        MX_THROW("Specified edge has no opposite.");
    }

    Side operator ~(Side const side)
    {
        switch (side)
        {
        case INNER: return OUTER;
        case OUTER: return INNER;
        }
        MX_THROW("Invalid side specified.");
    }

    Dimension edgeToDimension(Edge const edge)
    {
        switch (edge)
        {
        case LEFT: case RIGHT: return WIDTH;
        case TOP: case BOTTOM: return HEIGHT;
        }
        MX_THROW("Invalid edge specified.");
    }

    Measure Measure::Parse(std::string_view const & specv)
    {
        auto const spec = std::string{ specv };
        constexpr static auto const pattern = R"(^([0-9]+(?:\.?[0-9]+)?)(em|px|pt|%)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex))
        {
            // TODO "entangled" => max of all elements with same class
            MX_THROW(std::format("Invalid size: bad format. Expected N[.F][px|em|pt|%], saw {}", spec).c_str());
        }

        double offset = atof(matches[1].str().c_str());
        std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
        auto unit = Unit::PX;
        if (unitStr == "em") unit = EM;
        else if (unitStr == "%") unit = PC;
        else if (unitStr == "pt") unit = PT;

        return { offset, unit };
    }

    std::optional<int> Measure::toPixels(CaelusElement const * context, Dimension const dim, Side const side) const
    {
        switch (unit)
        {
        case PX:
            return (int)value;

        case EM:
            return static_cast<int>(static_cast<double>(getFontHeight(context->getHwnd())) * value));

        case PT:
        {
            auto hdc = GetDC(context->getInnerHwnd());
            return MulDiv((int)value, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        }

        case PC:
            if (context->m_futureRect[side].size[dim].has_value())
            {
                return ((double)(context->m_futureRect[side].size[dim].value())) * value;
            }
            return std::nullopt;
        }

        MX_THROW("Unsupported unit for conversion to pixels.");
    }

    int ResolvedRect::GetEdge(Edge const edge) const
    {
        if (m_edge[edge].has_value()) return m_edge[edge].value();
        MX_THROW("Unresolved edge");
    }

    int ResolvedRect::GetSize(Dimension const dim) const
    {
        if (m_size[dim].has_value()) return m_size[dim].value();
        MX_THROW("Unresolved dimension");
    }

    bool ResolvedRect::HasEdge(Edge const edge) const
    {
        return m_edge[edge].has_value();
    }

    bool ResolvedRect::HasSize(Dimension const dim) const
    {
        return m_size[dim].has_value();
    }

    void ResolvedRect::SetEdge(Edge const edge, int const px)
    {
        switch (edge)
        {
        case TOP: SetTop(px); return;
        case LEFT: SetLeft(px); return;
        case BOTTOM: SetBottom(px); return;
        case RIGHT: SetRight(px); return;
        }
        MX_THROW("Invalid edge");
    }

    void ResolvedRect::SetSize(Dimension const dim, int const px)
    {
        switch (dim)
        {
        case WIDTH: SetWidth(px); return;
        case HEIGHT: SetHeight(px); return;
        }
        MX_THROW("Invalid dimension");
    }

    void ResolvedRect::SetInnerSize(Dimension const dim, int const px)
    {
        switch (dim)
        {
        case WIDTH: SetInnerWidth(px); return;
        case HEIGHT: SetInnerHeight(px); return;
        }
        MX_THROW("Invalid dimension");
    }

    void ResolvedRect::SetInnerHeight(int const px)
    {
        m_innerSize[HEIGHT] = px;
        if (m_padd[TOP].has_value() && m_padd[BOTTOM].has_value())
        {
            SetHeight(px + m_padd[TOP].value() + m_padd[BOTTOM].value());
        }
    }

    void ResolvedRect::SetInnerWidth(int const px)
    {
        m_innerSize[WIDTH] = px;
        if (m_padd[LEFT].has_value() && m_padd[RIGHT].has_value())
        {
            SetWidth(px + m_padd[LEFT].value() + m_padd[RIGHT].value());
        }
    }

    void ResolvedRect::SetTop(int const px)
    {
        m_edge[TOP] = px;
        if (m_size[HEIGHT].has_value() && !m_edge[BOTTOM].has_value())
        {
            m_edge[BOTTOM] = px + m_size[HEIGHT].value();
        }
        else if (m_edge[BOTTOM].has_value())
        {
            SetHeight(m_edge[BOTTOM].value() - px);
        }
    }

    void ResolvedRect::SetBottom(int const px)
    {
        m_edge[BOTTOM] = px;
        if (m_size[HEIGHT].has_value() && !m_edge[TOP].has_value())
        {
            m_edge[TOP] = px - m_size[HEIGHT].value();
        }
        else if (m_edge[TOP].has_value())
        {
            SetHeight(px - m_edge[TOP].value());
        }
    }

    void ResolvedRect::SetHeight(int const px)
    {
        m_size[HEIGHT] = px;
        if (m_edge[TOP].has_value())
        {
            m_edge[BOTTOM] = m_edge[TOP].value() + px;
        }
        else if (m_edge[BOTTOM].has_value())
        {
            m_edge[TOP] = m_edge[BOTTOM].value() - px;
        }
        if (m_padd[TOP].has_value() && m_padd[BOTTOM].has_value())
        {
            m_innerSize[HEIGHT] = px - m_padd[TOP].value() - m_padd[BOTTOM].value();
        }
    }

    void ResolvedRect::SetLeft(int const px)
    {
        m_edge[LEFT] = px;
        if (m_size[WIDTH].has_value() && !m_edge[RIGHT].has_value())
        {
            m_edge[RIGHT] = px + m_size[WIDTH].value();
        }
        else if (m_edge[RIGHT].has_value())
        {
            SetWidth(m_edge[RIGHT].value() - px);
        }
    }

    void ResolvedRect::SetRight(int const px)
    {
        m_edge[RIGHT] = px;
        if (m_size[WIDTH].has_value() && !m_edge[LEFT].has_value())
        {
            m_edge[LEFT] = px - m_size[WIDTH].value();
        }
        else if (m_edge[LEFT].has_value())
        {
            SetWidth(px - m_edge[LEFT].value());
        }
    }

    void ResolvedRect::SetWidth(int const px)
    {
        m_size[WIDTH] = px;
        if (m_edge[LEFT].has_value())
        {
            m_edge[RIGHT] = m_edge[LEFT].value() + px;
        }
        else if (m_edge[RIGHT].has_value())
        {
            m_edge[LEFT] = m_edge[RIGHT].value() - px;
        }
        if (m_padd[LEFT].has_value() && m_padd[RIGHT].has_value())
        {
            m_innerSize[WIDTH] = px - m_padd[LEFT].value() - m_padd[RIGHT].value();
        }
    }

    size_t ResolvedRect::CountUnresolved() const
    {
        return !m_edge[TOP].has_value() + !m_edge[LEFT].has_value() + !m_edge[BOTTOM].has_value() + !m_edge[RIGHT].has_value()
            + !m_size[WIDTH].has_value() + !m_size[HEIGHT].has_value();
    }

}