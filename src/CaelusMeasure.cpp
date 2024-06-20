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
        return axis == Axis::X ? r.left : r.top;
    }
    */

    std::string edgeToKeyword(Edge const edge)
    {
        switch (edge)
        {
        case Edge::TOP: return "top";
        case Edge::LEFT: return "left";
        case Edge::BOTTOM: return "bottom";
        case Edge::RIGHT: return "right";
        case Edge::ALL: return "center";
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

    int getFontHeight(HWND hwnd)
    {
        TEXTMETRIC tm = {};
        auto const hdc = GetDC(hwnd);
        auto const r = GetTextMetrics(hdc, &tm);
        ReleaseDC(hwnd, hdc);
        return tm.tmHeight;
    }

    int getLineHeight(HWND hwnd)
    {
        OUTLINETEXTMETRIC tm = {};
        auto const hdc = GetDC(hwnd);
        auto const r = GetOutlineTextMetrics(hdc, sizeof(OUTLINETEXTMETRIC), &tm);
        ReleaseDC(hwnd, hdc);
        auto lineHeight = (-tm.otmDescent) + tm.otmLineGap + tm.otmAscent;
        return MulDiv(lineHeight, getDpi(hwnd), 96);
    }

    Edge keywordToEdge(std::string_view const & keyword)
    {
        switch (keyword[0])
        {
        case 't': return Edge::TOP;
        case 'l': return Edge::LEFT;
        case 'b': return Edge::BOTTOM;
        case 'r': return Edge::RIGHT;
        }
        MX_THROW("Unknown edge.");
    }

    bool isHEdge(Edge const edge)
    {
        return edge == Edge::LEFT || edge == Edge::RIGHT;
    }

    bool isVEdge(Edge const edge)
    {
        return edge == Edge::TOP || edge == Edge::BOTTOM;
    }

    bool isFarEdge(Edge const edge)
    {
        return edge == Edge::RIGHT || edge == Edge::BOTTOM;
    }

    Edge operator ~(Edge const edge)
    {
        switch (edge)
        {
        case Edge::TOP: return Edge::BOTTOM;
        case Edge::BOTTOM: return Edge::TOP;
        case Edge::LEFT: return Edge::RIGHT;
        case Edge::RIGHT: return Edge::LEFT;
        }
        MX_THROW("Specified edge has no opposite.");
    }

    Dimension edgeToDimension(Edge const edge)
    {
        switch (edge)
        {
        case Edge::LEFT: case Edge::RIGHT: return Dimension::WIDTH;
        case Edge::TOP: case Edge::BOTTOM: return Dimension::HEIGHT;
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
        size_t count = 0;
        count += !m_edge[TOP].has_value();
        count += !m_edge[LEFT].has_value();
        count += !m_edge[BOTTOM].has_value();
        count += !m_edge[RIGHT].has_value();
        count += !m_size[WIDTH].has_value();
        count += !m_size[HEIGHT].has_value();
        return count;
    }

}