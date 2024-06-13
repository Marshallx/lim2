#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "JamlColor.h"
#include "JamlFont.h"
#include "JamlMeasure.h"

namespace jaml
{
    enum JamlElementType
    {
        GENERIC = 0, TEXT, EDITBOX, BUTTON, LISTBOX, COMBOBOX, CHECKBOX, CLASS
    };

    class JamlClass
    {
    public:
        void SetBackgroundColor(Color const & color);
        void SetBackgroundColor(std::string_view const & color);
        void SetContentAlignmentH(Edge const edge);
        void SetContentAlignmentV(Edge const edge);
        void SetElementType(JamlElementType const v);
        void SetElementType(std::string_view const & v);
        void SetFontColor(Color const & color);
        void SetFontColor(std::string_view const & color);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(int const weight);
        void SetFontWeight(std::string_view const & weight);
        void SetHeight(std::string_view const & height);
        void SetImagePath(std::filesystem::path const & path);
        void SetLabel(std::string_view const & label);
        void SetOpacity(uint8_t const opacity);
        void SetPadding(Edge const edge, Measure const & v);
        void SetTether(Edge const mySide, std::string_view const & otherId, Edge const otherSide, Measure const & offset);
        void SetTether(Edge const mySide, std::string const & spec);
        void SetValue(std::string_view const & v);
        void SetVisible(bool const v);
        void SetWidth(std::string_view const & width);
        void hide();
        void show();


        std::optional<Edge> alignContentH;
        std::optional<Edge> alignContentV;
        std::optional<Color> backgroundColor;
        std::optional<Color> borderColor;
        std::optional<Measure> borderSize;
        std::optional<Measure> borderRadius;
        std::vector<std::string> classes;
        std::vector<std::string> children;
        JamlElementType elementType = GENERIC;
        std::optional<Font> font;
        std::filesystem::path imagePath;
        std::string label;
        std::string name;
        std::optional<uint8_t> opacity = 255;
        std::optional<Measure> padding[4];
        std::optional<Measure> size[2];
        std::optional<Tether> tethers[4];
        std::string value;
        bool visible = true;
    };
}
