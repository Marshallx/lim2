#include <regex>

#include "JamlClass.h"
#include "MxiLogging.h"

namespace jaml
{
    void JamlClass::SetBackgroundColor(Color const & color)
    {
        backgroundColor = color;
    }

    void JamlClass::SetBackgroundColor(std::string_view const & color)
    {
        SetBackgroundColor(Color::Parse(color));
    }

    void JamlClass::SetContentAlignmentH(Edge const edge)
    {
        alignContentH = edge;
    }

    void JamlClass::SetContentAlignmentV(Edge const edge)
    {
        alignContentV = edge;
    }

    void JamlClass::SetElementType(JamlElementType const v)
    {
        elementType = v;
    }

    void JamlClass::SetElementType(std::string_view const & v)
    {
        if (v == "button") { elementType = BUTTON; return; }
        if (v == "combobox") { elementType = COMBOBOX; return; }
        if (v == "edit") { elementType = EDITBOX; return; }
        if (v == "listbox") { elementType = LISTBOX; return; }
        if (v == "checkbox") { elementType = CHECKBOX; return; }
        if (v == "class") { elementType = CLASS; return; }
        elementType = GENERIC;
    }

    void JamlClass::SetFontColor(Color const & color)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetColor(color);
    }

    void JamlClass::SetFontColor(std::string_view const & color)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetColor(color);
    }

    void JamlClass::SetFontFace(std::string_view const & face)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetFace(face);
    }

    void JamlClass::SetFontSize(std::string_view const & size)
    {
        SetFontSize(size);
    }

    void JamlClass::SetFontStyle(std::string_view const & style)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetStyle(style);
    }

    void JamlClass::SetFontWeight(std::string_view const & weight)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetWeight(weight);
    }

    void JamlClass::SetFontWeight(int const weight)
    {
        if (!font.has_value()) font = Font{};
        font.value().SetWeight(weight);
    }

    void JamlClass::SetHeight(std::string_view const & height)
    {
        size[HEIGHT] = Measure::Parse(height);
    }

    void JamlClass::SetImagePath(std::filesystem::path const & path)
    {
        imagePath = path;
    }

    void JamlClass::SetLabel(std::string_view const & v)
    {
        label = v;
    }

    void JamlClass::SetOpacity(uint8_t const v)
    {
        opacity = v;
    }

    void JamlClass::SetPadding(Edge const edge, Measure const & v)
    {
        padding[edge] = v;
    }

    void JamlClass::SetValue(std::string_view const & v)
    {
        value = v;
    }

    void JamlClass::SetVisible(bool const v)
    {
        visible = v;
    }
    void JamlClass::hide() { SetVisible(false); }
    void JamlClass::show() { SetVisible(true); }

    void JamlClass::SetWidth(std::string_view const & width)
    {
        if (width == "auto") size[WIDTH].reset();
        else size[WIDTH] = Measure::Parse(width);
    }

    void JamlClass::SetTether(Edge const mySide, std::string_view const & otherId, Edge const otherSide, Measure const & offset)
    {
        tethers[mySide] = { otherId, otherSide, offset };
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

            tethers[mySide] = { matches[1].str(), otherSide, { offset, unit } };
            // TODO Validate that no tether has cyclic dependencies
        }
        else if (parent && std::regex_search(spec, matches, simpleRegex))
        {
            double offset = atof(matches[1].str().c_str());
            if (mySide == BOTTOM || mySide == RIGHT) offset = -offset;
            std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") MX_THROW("Invalid tether: % not implemented.");
            tethers[mySide] = { parent->id, mySide, { offset, unit } };
        }
        else MX_THROW(std::format("Invalid tether: bad format. Expected [id>side][±offset em|px], saw {}", spec).c_str());
    }
}