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
        friend Measure;
    public:
        // Painting
        static void Register(HINSTANCE hInstance);
        LRESULT paint(HWND hwnd, HDC hdc);
        HBRUSH GetBackgroundBrush();
        HFONT GetHfont();
        HBRUSH GetBorderBrush();
        void DrawBorder(HDC hdc);

        // Element arrangement
        CaelusElement(std::string_view const & name) : m_name(std::string{ name })
        {
            if (name.empty()) MX_THROW("All elements require a unique name");
        };
        CaelusElement * FindElement(std::string_view const & name);
        CaelusElement * AppendChild(std::string_view const & name);
        CaelusElement * InsertChild(std::string_view const & name, size_t n);
        void Remove();
        void RemoveChild(size_t const n);
        void RemoveChildren();
        void show();
        void hide();

        // Style getters
        auto const & GetBackgroundColor() const;
        CaelusElement * GetChild(size_t const i);
        HWND GetHwnd() const noexcept;
        CaelusElement * GetParent() noexcept;
        CaelusElementType GetType() const;
        std::string const & GetValue() const;
        CaelusWindow const * GetWindow() const;

        // Style setters
        void SetBackgroundColor(Color const & v);
        void SetBackgroundColor(std::string const & spec);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(int const weight);
        void SetHeight(std::string const & spec);
        void SetImage(HBITMAP v);
        void SetLabel(std::string_view const & label);
        void SetOpacity(uint8_t const opacity);
        void SetOpacity(double const opacity);
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

    protected:
        void Build();
        void Spawn(HINSTANCE hInstance);
        void PrepareToComputeLayout();
        void UpdateBackgroundBrush();
        void UpdateFont();

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t ComputeLayout();

        Resolved ComputeEdge(Edge const edge);
        Resolved ComputePadding(Edge const edge);
        Resolved ComputeBorder(Edge const edge);
        Resolved ComputeNC(Edge const edge);
        Resolved ComputeSize(Dimension const dim);
        std::optional<int> MeasureToPixels(Measure const * measure, Dimension const dim) const;

        // Move futureRect to currentRect and redraw everything
        void CommitLayout(HINSTANCE hInstance);

        static Tether const GetDefaultTether(Edge const edge);
        HFONT GetHfont();
        CaelusElement * GetSibling(std::string_view const & name) const;
        CaelusElement * GetSibling(Edge const edge) const;

        // Styles inherited from parent or classes
        auto const & GetBorderColor() const;
        auto const & GetBorderWidthDef(Edge const edge) const;
        auto const & GetLabel() const;
        auto const & GetFontFace() const;
        auto const & GetFontItalic() const;
        auto const & GetFontSize() const;
        auto const & GetFontWeight() const;
        auto const & GetPaddingDef(Edge const edge) const;
        auto const & GetSizeDef(Dimension const dim) const;
        auto const & GetTether(Edge const edge) const;
        auto const & GetTextAlignH() const;
        auto const & GetTextColor() const;

        std::vector<std::shared_ptr<CaelusElement>> m_children = {};
        CaelusClass * m_class = nullptr;
        std::string m_name;
        CaelusElement * m_parent = nullptr;
        ResolvedRect m_currentRect;
        ResolvedRect m_futureRect;
        HFONT m_hfont = NULL;
        HWND m_hwnd = 0;

    protected:
        CaelusElement() = default;
    };
}