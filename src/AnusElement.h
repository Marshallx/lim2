#pragma once

#include <Windows.h>

#include "AnusClass.h"
#include "AnusWindow.h"
#include "MxiLogging.h"

namespace Anus
{

    LRESULT AnusElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    class AnusElement
    {
    public:
        AnusElement(std::string_view const & name) : m_name(std::string{ name })
            { if (name.empty()) MX_THROW("All elements require a unique name"); };

        static void registerClass(HINSTANCE hInstance);
        LRESULT paint(HWND hwnd, HDC hdc);

        Element * addChild(std::string_view const & id = {});

        Element * getChild(size_t const i) const;
        Element * getParent(bool const returnSelfIfRoot = true) noexcept;
        Element * findElement(std::string_view const & id);
        Window * getRoot() const noexcept;

        void removeChildren();
        void remove();

        HBRUSH getBackgroundBrush() const noexcept;
        Color const & getBackgroundColor() const noexcept;
        HWND getOuterHwnd() const noexcept;
        HWND getInnerHwnd() const noexcept;
        HFONT getFont() const;
        AnusElementType getType() const;
        std::string const & getFontFace() const;
        Measure const & getFontSize() const;
        FontStyle const & getFontStyle() const;
        int getFontWeight() const;
        Color getTextColor() const noexcept;
        std::string const & getValue() const;

        static Measure parseMeasure(std::string const & spec);

        void setBackgroundColor(Color const & v);
        void setBackgroundColor(std::string const & spec);
        void setFontFace(std::string_view const & face);
        void setFontSize(Measure const & size);
        void setFontSize(std::string const & spec);
        void setFontStyle(FontStyle const & style);
        void setFontWeight(int const weight);
        void setHeight(std::string const & spec);
        void setId(std::string_view const & v);
        void setImage(HBITMAP v);
        void setLabel(std::string_view const & v);
        void setOpacity(uint8_t const v);
        void setPadding(Edge const edge, Measure const & v);
        void setTextAlignH(Edge const v);
        void setTextColor(Color const & v);
        void setTextColor(std::string const & spec);
        void setType(AnusElementType const v);
        void setType(std::string_view const & v);
        void setValue(std::string_view const & v);
        void setVisible(bool const v = true);
        void setWidth(std::string const & spec);

        void tether(Edge const myEdge, std::string_view const & otherId, Edge const otherEdge, Measure const & offset);
        void tether(Edge const myEdge, std::string const & spec);

        void updateBackgroundBrush();
        void updateFont();

        void show();
        void hide();

        // Applies a tether offset to an otherwise resolved coordinate. Returns 1 if offset is unresolved (and resets the coord), 0 otherwise.
        Resolved applyOffset(Side const side, Edge const edge, Measure const & offset);

    protected:
        void Build();
        void PrepareToComputeLayout();

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t ComputeLayout();

        Resolved ComputeEdge(Edge const edge);
        Resolved ComputeDimension(Dimension const dim);

        // Move futureRect to currentRect and redraw everything
        void CommitLayout();

        AnusElement * GetSibling(std::string_view const & name) const;
        AnusElement * GetSibling(Edge const edge) const;
        static Tether const GetDefaultTether(Edge const edge);
        Tether const * GetTether(Edge const edge) const;
        AnusWindow const * GetWindow() const;
        std::vector<std::shared_ptr<AnusElement>> m_children = {};
        std::string m_name;
        AnusElement * m_parent = nullptr;
        AnusClass * m_class = nullptr;
        ResolvedRect m_currentRect;
        ResolvedRect m_futureRect;

        HBRUSH backgroundBrush = NULL;
        bool created = false;
        std::string fontFace;
        FontStyle fontStyle = FontStyle::INHERIT;
        int fontWeight = 0;
        Measure fontSize = { 0, Unit::NONE };
        HFONT font = 0;
        Edge textAlignH = LEFT;
        HWND hwndInner = 0;
        HWND hwndOuter = 0;
        size_t i = 0;
        HBITMAP image = 0;
        std::string label;
        uint8_t opacity = 255;
        Measure padding[4];
        Measure size[2] = { {0, NONE}, {0, NONE} };
        Tether tethers[4];
        Color textColor;
        AnusElementType type = AnusElementType::GENERIC;
        std::string value;
        bool visible = true;

    protected:
        Element() {};
    };
}