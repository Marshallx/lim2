#pragma once

#include <Windows.h>

#include "CaelusClass.h"
#include "CaelusWindow.h"
#include "MxiLogging.h"

namespace Caelus
{

    LRESULT CaelusElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    class CaelusElement
    {
    public:
        struct
        {
            HBRUSH GetBackgroundBrush() const noexcept;
        } m_drawing;

        CaelusElement(std::string_view const & name) : m_name(std::string{ name })
            { if (name.empty()) MX_THROW("All elements require a unique name"); };

        static void registerClass(HINSTANCE hInstance);
        LRESULT paint(HWND hwnd, HDC hdc);

        CaelusElement * FindElement(std::string_view const & name);
        Color const * GetBackgroundColor() const noexcept;
        CaelusElement * GetChild(size_t const i);
        HWND GetHwnd() const noexcept;
        Font GetFont() const;
        CaelusElement * GetParent() noexcept;
        CaelusElementType GetType() const;
        std::string const & GetValue() const;
        CaelusWindow const * GetWindow() const;

        CaelusElement * AppendChild(std::string_view const & name);
        CaelusElement * InsertChild(std::string_view const & name, size_t n);
        void Remove();
        void RemoveChild(size_t const n);
        void RemoveChildren();

        void SetBackgroundColor(Color const & v);
        void SetBackgroundColor(std::string const & spec);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(Measure const & size);
        void SetFontSize(std::string const & spec);
        void SetFontStyle(FontStyle const & style);
        void SetFontWeight(int const weight);
        void SetHeight(std::string const & spec);
        void SetId(std::string_view const & v);
        void SetImage(HBITMAP v);
        void SetLabel(std::string_view const & v);
        void SetOpacity(uint8_t const v);
        void SetPadding(Edge const edge, Measure const & v);
        void SetTextAlignH(Edge const v);
        void SetTextColor(Color const & v);
        void SetTextColor(std::string const & spec);
        void SetType(CaelusElementType const v);
        void SetType(std::string_view const & v);
        void SetValue(std::string_view const & v);
        void SetVisible(bool const v = true);
        void SetWidth(std::string const & spec);

        void tether(Edge const myEdge, std::string_view const & otherId, Edge const otherEdge, Measure const & offset);
        void tether(Edge const myEdge, std::string const & spec);

        void updateBackgroundBrush();
        void updateFont();

        void show();
        void hide();

    protected:
        void Build();
        void PrepareToComputeLayout();

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t ComputeLayout();

        Resolved ComputeEdge(Edge const edge);
        Resolved ComputePadding(Edge const edge);
        Resolved ComputeBorder(Edge const edge);
        Resolved ComputeNC(Edge const edge);
        Resolved ComputeSize(Dimension const dim);

        // Move futureRect to currentRect and redraw everything
        void CommitLayout();

        Measure const * GetBorderWidthDef(Edge const edge) const;
        static Tether const GetDefaultTether(Edge const edge);
        Measure const * GetPaddingDef(Edge const edge) const;
        CaelusElement * GetSibling(std::string_view const & name) const;
        CaelusElement * GetSibling(Edge const edge) const;
        Measure const * GetSizeDef(Dimension const dim) const;
        Tether const * GetTether(Edge const edge) const;

        std::vector<std::shared_ptr<CaelusElement>> m_children = {};
        CaelusClass * m_class = nullptr;
        std::string m_name;
        CaelusElement * m_parent = nullptr;
        ResolvedRect m_currentRect;
        ResolvedRect m_futureRect;
        HWND m_hwnd = 0;

    protected:
        CaelusElement() = default;
    };
}