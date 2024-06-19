#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "CaelusColor.h"
#include "CaelusFont.h"
#include "CaelusMeasure.h"
#include "MxiLogging.h"

namespace Caelus
{
    enum CaelusElementType
    {
        GENERIC = 0, TEXT, EDITBOX, BUTTON, LISTBOX, COMBOBOX, CHECKBOX, CLASS
    };

    class CaelusElement;
    class CaelusWindow;


    class CaelusClass
    {
        friend CaelusWindow;

    public:
        CaelusClass(std::string_view const & name) : m_name(name)
            { if (name.empty()) MX_THROW("All classes and elements require a unique name"); };

        void AddClassNames(std::string_view const & classes);
        Color const * GetBackgroundColor() const;
        Measure const * GetBorderWidth(Edge const edge) const;
        std::vector<std::string> const & GetClassNames() const;
        CaelusElement const * GetElement() const noexcept;
        std::string const & GetName() const noexcept;
        std::string const & GetParentName() const noexcept;
        Tether const * GetTether(Edge const edge) const;
        Measure const * GetPadding(Edge const edge) const;

        void SetBackgroundColor(Color const & color);
        void SetBackgroundColor(std::string_view const & color);
        void SetBackgroundColor(uint32_t const color);
        void SetBorder(std::string const & spec, Edge const edge = Edge::ALL);
        void SetBorderColor(Color const & color, Edge const edge = Edge::ALL );
        void SetBorderColor(std::string_view const & color, Edge const edge = Edge::ALL);
        void SetBorderColor(uint32_t const color, Edge const edge = Edge::ALL);
        void SetBorderRadius(std::string_view const & radius, Corner const corner = Corner::ALL);
        void SetBorderWidth(std::string_view const & width, Edge const edge = Edge::ALL);
        void SetContentAlignmentH(Edge const edge);
        void SetContentAlignmentV(Edge const edge);
        void SetElement(CaelusElement * element);
        void SetElementType(std::string_view const & type);
        void SetFontColor(Color const & color);
        void SetFontColor(std::string_view const & color);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(std::string_view const & weight);
        void SetHeight(std::string_view const & height);
        void SetImagePath(std::filesystem::path const & path);
        void SetLabel(std::string_view const & label);
        void SetOpacity(uint8_t const opacity);
        void SetPadding(std::string_view const & padding, Edge const edge = Edge::ALL);
        void SetParentName(std::string_view const & parent);
        void SetTether(Edge const mySide, std::string_view const & otherId,
            Edge const otherSide, Measure const & offset);
        void SetTether(Edge const mySide, std::string_view const & tether);
        void SetValue(std::string_view const & v);
        void SetVisible(bool const v);
        void SetWidth(std::string_view const & width);
        void hide();
        void show();

    private:
        CaelusClass() = delete;

        std::optional<Edge> m_alignContentH;
        std::optional<Edge> m_alignContentV;
        std::optional<Color> m_backgroundColor;
        std::optional<Color> m_borderColor[4];
        std::optional<Measure> m_borderRadius[4];
        //std::optional<BorderStyle> m_borderStyle[4];
        std::optional<Measure> m_borderWidth[4];
        std::vector<std::string> m_classNames = {};
        CaelusElement * m_element = nullptr;
        CaelusElementType m_elementType = GENERIC;
        std::optional<Font> m_font;
        std::filesystem::path m_imagePath;
        std::string m_label;
        std::string m_name;
        std::optional<uint8_t> m_opacity = 255;
        std::optional<Measure> m_padding[4];
        std::string m_parentName;
        std::optional<Measure> m_size[2];
        std::optional<Tether> m_tethers[4];
        std::string m_value;
        bool m_visible = true;
    };

    class CaelusClassMap
    {
    public:
        Color const * GetBackgroundColor(std::string_view const & name) const;
        Measure const * GetBorderWidth(Edge const edge, std::string_view const & name) const;
        CaelusClass * GetClass(std::string_view const & name) const;
        void GetClassChain(std::string_view const & name, std::vector<CaelusClass *> & chain) const;
        Measure const * GetPadding(Edge const edge, std::string_view const & name) const;
        Tether const * GetTether(Edge const edge, std::string_view const & name) const;
        std::unordered_map<std::string, std::shared_ptr<CaelusClass>> m_map = {};
    };
}
