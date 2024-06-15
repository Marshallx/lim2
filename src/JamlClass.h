#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "JamlColor.h"
#include "JamlFont.h"
#include "JamlMeasure.h"
#include "MxiLogging.h"

namespace jaml
{
    enum JamlElementType
    {
        GENERIC = 0, TEXT, EDITBOX, BUTTON, LISTBOX, COMBOBOX, CHECKBOX, CLASS
    };

    class JamlElement;
    class JamlWindow;


    class JamlClass
    {
        friend JamlWindow;

    public:
        JamlClass(std::string_view const & name) : name(name)
            { if (name.empty()) MX_THROW("All classes and elements require a unique name"); };

        void AddClassNames(std::string_view const & classes);
        std::vector<std::string> const & GetClassNames() const;
        JamlElement * const & GetElement() const noexcept;
        std::string const & GetName() const noexcept;
        std::string const & GetParentName() const noexcept;
        std::optional<Tether> const & GetTether(Edge const edge) const;

        void SetBackgroundColor(Color const & color);
        void SetBackgroundColor(std::string_view const & color);
        void SetBackgroundColor(uint32_t const color);
        void SetContentAlignmentH(Edge const edge);
        void SetContentAlignmentV(Edge const edge);
        void SetElement(JamlElement * element);
        void SetElementType(JamlElementType const type);
        void SetElementType(std::string_view const & type);
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
        void SetParentName(std::string_view const & parent);
        void SetTether(Edge const mySide, std::string_view const & otherId,
            Edge const otherSide, Measure const & offset);
        void SetTether(Edge const mySide, std::string const & spec);
        void SetValue(std::string_view const & v);
        void SetVisible(bool const v);
        void SetWidth(std::string_view const & width);
        void hide();
        void show();

    protected:
        JamlClass() = delete;

        std::optional<Edge> alignContentH;
        std::optional<Edge> alignContentV;
        std::optional<Color> backgroundColor;
        std::optional<Color> borderColor;
        std::optional<Measure> borderSize;
        std::optional<Measure> borderRadius;
        std::vector<std::string> classes = {};
        JamlElement * element = nullptr;
        JamlElementType elementType = GENERIC;
        std::optional<Font> font;
        std::filesystem::path imagePath;
        std::string label;
        std::string name;
        std::optional<uint8_t> opacity = 255;
        std::optional<Measure> padding[4];
        std::string parentName;
        std::optional<Measure> size[2];
        std::optional<Tether> tethers[4];
        std::string value;
        bool visible = true;
    };

    using ClassMap = std::unordered_map<std::string, std::shared_ptr<JamlClass>>;
}
