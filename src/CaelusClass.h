#pragma once

#include <assert.h>
#include <filesystem>
#include <string>
#include <vector>

#include "CaelusColor.h"
#include "CaelusMeasure.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Caelus
{
    enum InputType
    {
        NONE, EDITBOX, BUTTON, CHECKBOX, LISTBOX, COMBOBOX, RADIO
    };

    enum CaelusElementStyle
    {
        BACKGROUND_COLOR,
        BORDER_COLOR,
        BORDER_RADIUS,
        BORDER_WIDTH,
        FONT_FACE,
        FONT_ITALIC,
        FONT_SIZE,
        FONT_WEIGHT,
        INPUT_TYPE,
        LABEL,
        PADDING,
        SIZE,
        TETHER,
        TEXT_ALIGNH,
        TEXT_ALIGNV,
        TEXT_COLOR
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
    class CaelusClass;

    class CaelusClassMap
    {
    public:
        CaelusClass * GetClass(std::string_view const & name) const;
        void GetClassChain(std::string_view const & name, std::vector<CaelusClass *> & chain) const;
        std::unordered_map<std::string, std::shared_ptr<CaelusClass>> m_map = {};
    };

    class CaelusClass
    {
    public:
        CaelusClass(std::string_view const & name, CaelusClassMap * map) : m_name(name), m_map(map)
            { if (name.empty()) MX_THROW("All classes and elements require a unique name"); };

        void AddClassNames(std::string_view const & classes);

        std::vector<std::string> const & GetClassNames() const { return m_classNames; };
        CaelusElement const * GetElement() const noexcept { return m_element; };
        std::string const & GetName() const noexcept { return m_name; };
        std::string const & GetParentName() const noexcept { return m_parentName; };

        template<typename T, typename N>
        std::optional<T> const & GetStyle(CaelusElementStyle const style, N const edge, bool considerSuperClasses = true) const
        {
            if (considerSuperClasses)
            {
                auto const cs = GetClassChain();
                for (auto const c : cs)
                {
                     auto const & v = c->GetStyle<T>(style, edge, false);
                     if (v.has_value()) return v;
                }
            }
            else
            {
                if constexpr (std::is_same_v<T, Color>)
                {
                    switch (style)
                    {
                    case BACKGROUND_COLOR: return m_backgroundColor;
                    case BORDER_COLOR: return m_borderColor[edge];
                    case TEXT_COLOR: return m_textColor;
                    }
                }
                else if constexpr (std::is_same_v<T, Measure>)
                {
                    switch (style)
                    {
                    case BORDER_RADIUS: return m_borderRadius[edge];
                    case BORDER_WIDTH: return m_borderWidth[edge];
                    case FONT_SIZE: return m_fontSize;
                    case PADDING: return m_padding[edge];
                    case SIZE: return m_size[edge];
                    }
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    switch (style)
                    {
                    case FONT_FACE: return m_fontFace;
                    case LABEL: return m_label;
                    }
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    switch (style)
                    {
                    case FONT_ITALIC: return m_fontItalic;
                    }
                }
                else if constexpr (std::is_same_v<T, int>)
                {
                    switch (style)
                    {
                    case FONT_WEIGHT: return m_fontWeight;
                    }
                }
                else if constexpr (std::is_same_v<T, Tether>)
                {
                    switch (style)
                    {
                    case TETHER: return m_tethers[edge];
                    }
                }
                else if constexpr (std::is_same_v<T, Edge>)
                {
                    switch (style)
                    {
                    case TEXT_ALIGNH: return m_alignTextH;
                    case TEXT_ALIGNV: return m_alignTextV;
                    }
                }
                else if constexpr (std::is_same_v<T, InputType>)
                {
                    switch (style)
                    {
                    case INPUT_TYPE: return m_inputType;
                    }
                }
                else
                {
                    static_assert(mxi::always_false<T>, "Unsupported type for GetStyle()");
                }
            }
            static std::optional<T> def{};
            return def;
        }

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
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(std::string_view const & weight);
        void SetFontWeight(int const weight);
        void SetHeight(std::string_view const & height);
        void SetImagePath(std::filesystem::path const & path);
        void SetInputType(std::string_view const & type);
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
        std::optional<std::string> m_fontFace;
        std::optional<InputType> m_inputType = NONE;
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

        CaelusClassMap * m_map = nullptr;

        std::vector<CaelusClass *> GetClassChain() const;
    };

}
