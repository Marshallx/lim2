#include <regex>

#include "JamlFont.h"
#include "MxiLogging.h"

#include "JamlMeasure.h"


namespace jaml
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

    std::string edgeToString(Edge const edge)
    {
        switch (edge)
        {
        case TOP: return "top";
        case LEFT: return "left";
        case BOTTOM: return "bottom";
        case RIGHT: return "right";
        case CENTER: return "center";
        case AUTO: return "auto";
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
        if (!std::regex_search(spec, matches, regex)) MX_THROW(std::format("Invalid size: bad format. Expected N[.F][px|em|pt|%], saw {}", spec).c_str());

        double offset = atof(matches[1].str().c_str());
        std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
        auto unit = Unit::PX;
        if (unitStr == "em") unit = EM;
        else if (unitStr == "%") unit = PC;
        else if (unitStr == "pt") unit = PT;

        return { offset, unit };
    }

    std::optional<int> Measure::toPixels(JamlElement const * context, Dimension const dim, Side const side) const
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
            if (context->futurePos[side].size[dim].has_value())
            {
                return ((double)(context->futurePos[side].size[dim].value())) * value;
            }
            return std::nullopt;
        }

        MX_THROW("Unsupported unit for conversion to pixels.");
    }

}