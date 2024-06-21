#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "CaelusColor.h"
#include "CaelusMeasure.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Caelus
{
    enum CaelusElementType
    {
        GENERIC = 0, TEXT, EDITBOX, BUTTON, LISTBOX, COMBOBOX, CHECKBOX, CLASS
    };

    enum FontWeight : int
    {
        THIN = 100,
        EXTRALIGHT = 200,
        LIGHT = 300,
        REGULAR = 400,
        MEDIUM = 500,
        SEMIBOLD = 600,
        BOLD = 700,
        EXTRABOLD = 800,
        BLACK = 900
    };

    class CaelusElement;

    class CaelusClass
    {
    public:
        CaelusClass(std::string_view const & name) : m_name(name)
            { if (name.empty()) MX_THROW("All classes and elements require a unique name"); };

        void AddClassNames(std::string_view const & classes);
        std::optional<Color> const & GetBackgroundColor() const noexcept { return m_backgroundColor; };
        std::optional<Color> const & GetBorderColor(Edge const edge) const noexcept { return m_borderColor[edge]; };
        std::optional<Measure> const & GetBorderWidthDef(Edge const edge) const noexcept { return m_borderWidth[edge]; };
        std::vector<std::string> const & GetClassNames() const { return m_classNames; };
        CaelusElement const * GetElement() const noexcept { return m_element; };
        std::optional<std::string> const & GetFontFace() const noexcept { return m_fontFace; };
        std::optional<bool> const & GetFontItalic() const noexcept { return m_fontItalic; };
        std::optional<Measure> const & GetFontSize() const noexcept { return m_fontSize; };
        std::optional<int> const & GetFontWeight() const noexcept { return m_fontWeight; };
        std::optional<std::string> const & GetLabel() const noexcept { return m_label; };
        std::string const & GetName() const noexcept { return m_name; };
        std::optional<Measure> const & GetPaddingDef(Edge const edge) const noexcept { return m_padding[edge]; };
        std::string const & GetParentName() const noexcept { return m_parentName; };
        std::optional<Measure> const & GetSizeDef(Dimension const dim) const noexcept { return m_size[dim]; };
        std::optional<Tether> const & GetTether(Edge const edge) const noexcept { return m_tethers[edge]; };
        std::optional<Edge> const & GetTextAlignH() const noexcept { return m_alignTextH; };
        std::optional<Color> const & GetTextColor() const noexcept { return m_textColor; };

        void SetBackgroundColor(Color const & color);
        void SetBackgroundColor(std::string_view const & color);
        void SetBackgroundColor(uint32_t const color);
        void SetBorder(std::string_view const & spec, Edge const edge = Edge::ALL_EDGES);
        void SetBorderColor(Color const & color, Edge const edge = Edge::ALL_EDGES );
        void SetBorderColor(std::string_view const & color, Edge const edge = Edge::ALL_EDGES);
        void SetBorderColor(uint32_t const color, Edge const edge = Edge::ALL_EDGES);
        void SetBorderRadius(std::string_view const & radius, Corner const corner = Corner::ALL_CORNERS);
        void SetBorderWidth(std::string_view const & width, Edge const edge = Edge::ALL_EDGES);
        void SetTextAlignH(Edge const edge);
        void SetTextAlignV(Edge const edge);
        void SetElement(CaelusElement * element);
        void SetElementType(std::string_view const & type);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(std::string_view const & weight);
        void SetFontWeight(int const weight);
        void SetHeight(std::string_view const & height);
        void SetImagePath(std::filesystem::path const & path);
        void SetLabel(std::string_view const & label);
        void SetOpacity(uint8_t const opacity);
        void SetPadding(std::string_view const & padding, Edge const edge = Edge::ALL_EDGES);
        void SetParentName(std::string_view const & parent);
        void SetTether(Edge const mySide, std::string_view const & otherId, Edge const otherSide, Measure const & offset);
        void SetTether(Edge const mySide, std::string_view const & tether);
        void SetTextColor(Color const & color);
        void SetTextColor(std::string_view const & color);
        void SetTextColor(uint32_t const color);
        void SetValue(std::string_view const & v);
        void SetVisible(bool const v);
        void SetWidth(std::string_view const & width);
        void hide();
        void show();

    private:
        CaelusClass() = delete;

        std::optional<Edge> m_alignTextH;
        std::optional<Edge> m_alignTextV;
        std::optional<Color> m_backgroundColor;
        std::optional<Color> m_borderColor[4];
        std::optional<Measure> m_borderRadius[4];
        //std::optional<BorderStyle> m_borderStyle[4];
        std::optional<Measure> m_borderWidth[4];
        std::vector<std::string> m_classNames = {};
        CaelusElement * m_element = nullptr; // TODO need this?
        std::optional<CaelusElementType> m_elementType;
        std::optional<std::string> m_fontFace;
        std::optional<bool> m_fontItalic;
        std::optional<Measure> m_fontSize;
        std::optional<int> m_fontWeight;
        std::filesystem::path m_imagePath;
        std::optional<std::string> m_label;
        std::string m_name;
        std::optional<uint8_t> m_opacity = 255;
        std::optional<Measure> m_padding[4];
        std::string m_parentName;
        std::optional<Measure> m_size[2];
        std::optional<Tether> m_tethers[4];
        std::optional<Color> m_textColor;
        std::string m_value;
        bool m_visible = true;
    };

    class CaelusClassMap
    {
    public:
        CaelusClass * GetClass(std::string_view const & name) const;
        void GetClassChain(std::string_view const & name, std::vector<CaelusClass *> & chain) const;
        std::unordered_map<std::string, std::shared_ptr<CaelusClass>> m_map = {};
    };
}
