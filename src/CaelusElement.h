#pragma once

#include <Windows.h>

#include "CaelusClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Caelus
{
    LRESULT CaelusElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    class CaelusWindow;

    class CaelusElement
    {
    public:
        // Painting
        LRESULT Paint(HWND hwnd, HDC hdc);
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        HFONT GetHfont();

        // Element arrangement
        CaelusElement(std::string_view const & name);
        CaelusElement * FindElement(std::string_view const & name);
        CaelusElement * AppendChild(std::string_view const & name);
        CaelusElement * InsertChild(std::string_view const & name, size_t n);
        void Remove();
        void RemoveChild(size_t const n);
        void RemoveChildren();
        void show();
        void hide();
        CaelusElement * GetChild(size_t const n) const noexcept;
        HWND GetHwnd() const noexcept;
        CaelusElement * GetParent() noexcept;
        CaelusWindow const * GetWindow() const;

        std::string const & GetValue() const;

        // Style getters
        template<typename T>
        std::optional<T> const & GetStyle(CaelusElementStyle style, int const edge = 0) const
        {
            auto elem = this;
            while (elem)
            {
                auto const & v = elem->m_class->GetStyle<T>(style, edge);
                if (v.has_value()) return v;

                switch (style)
                {
                case LABEL:
                case SIZE:
                case TETHER:
                    // Uninheritable
                    break;
                default:
                    elem = elem->m_parent;
                    continue;
                }
                break;
            }
            static std::optional<T> def{};
            return def;
        }

        Color const & GetBackgroundColor() const { return GetStyle<Color>(BACKGROUND_COLOR).value(); }
        Color const & GetBorderColor(Edge const edge) const { return GetStyle<Color>(BORDER_COLOR, edge).value(); }
        Color const & GetTextColor() const { return GetStyle<Color>(TEXT_COLOR).value(); }
        Measure const & GetBorderRadius(Corner const corner) const { return GetStyle<Measure>(BORDER_RADIUS, corner).value(); }
        Measure const & GetBorderWidth(Edge const edge) const { return GetStyle<Measure>(BORDER_WIDTH, edge).value(); }
        Measure const & GetFontSize() const { return GetStyle<Measure>(FONT_SIZE).value(); }
        InputType GetInputType() const { auto const & type = GetStyle<InputType>(INPUT_TYPE); return (type.has_value()) ? type.value() : NONE; }
        Measure const & GetPadding(Edge const edge) const { return GetStyle<Measure>(PADDING, edge).value(); }
        std::optional<Measure> const & GetSize(Dimension const dim) const { return GetStyle<Measure>(SIZE, dim); }
        std::optional<Tether> const & GetTether(Edge const edge) const { return GetStyle<Tether>(TETHER, edge); }
        Edge GetTextAlignH() const { return GetStyle<Edge>(TEXT_ALIGNH).value(); }
        Edge GetTextAlignV() const { return GetStyle<Edge>(TEXT_ALIGNV).value(); }
        std::string const & GetFontFace() const { return GetStyle<std::string>(FONT_FACE).value(); }
        std::optional<std::string> const & GetLabel() const { return GetStyle<std::string>(LABEL); }
        bool GetFontItalic() const { return GetStyle<bool>(FONT_ITALIC).value(); }
        int GetFontWeight() const { return GetStyle<int>(FONT_WEIGHT).value(); }

        static Tether const GetDefaultTether(Edge const edge) { return { ".", ~edge, { 0, PX } }; }

        // Style setters
        void SetBackgroundColor(Color const & color);
        void SetBackgroundColor(std::string_view const & color);
        void SetBorderColor(Color const & color);
        void SetBorderColor(std::string_view const & color);
        void SetFontFace(std::string_view const & face);
        void SetFontSize(std::string_view const & size);
        void SetFontStyle(std::string_view const & style);
        void SetFontWeight(int const weight);
        void SetImage(HBITMAP imageHandle);
        void SetInputType(std::string_view const & type);
        void SetLabel(std::string_view const & label);
        void SetOpacity(uint8_t const opacity);
        void SetOpacity(double const opacity);
        void SetPadding(Edge const edge, Measure const & padding);
        void SetSize(std::string_view const & width, std::string_view const & height);
        void SetTextAlignH(Edge const edge);
        void SetTextColor(Color const & color);
        void SetTextColor(std::string_view const & color);
        void SetType(std::string_view const & type);
        void SetValue(std::string_view const & value);
        void SetVisible(bool const visible = true);

        void tether(Edge const myEdge, std::string_view const & otherId, Edge const otherEdge, Measure const & offset);
        void tether(Edge const myEdge, std::string const & spec);

    protected:
        CaelusElement() = default;
        void Build();
        void Spawn(HINSTANCE hInstance, HWND outerWindow = NULL);
        void PrepareToComputeLayout();
        wchar_t const * GetWindowClass() const;
        void UpdateFont();

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t ComputeLayout();

        Resolved ComputeEdge(Edge const edge);
        Resolved ComputePadding(Edge const edge);
        Resolved ComputeBorder(Edge const edge);
        Resolved ComputeNC(Edge const edge);
        Resolved ComputeSize(Dimension const dim);
        std::optional<int> MeasureToPixels(Measure const & measure, Dimension const dim) const;

        // Move futureRect to currentRect and redraw everything
        void CommitLayout(HINSTANCE hInstance, HWND outerWindow = NULL);

        CaelusElement * GetSibling(std::string_view const & name) const;
        CaelusElement * GetSibling(Edge const edge) const;

        std::vector<std::shared_ptr<CaelusElement>> m_children = {};
        CaelusClass * m_class = nullptr;
        std::string m_name;
        CaelusElement * m_parent = nullptr;
        ResolvedRect m_currentRect;
        ResolvedRect m_futureRect;
        HFONT m_hfont = NULL;
        HWND m_hwnd = 0;
        WNDPROC m_originalWndProc = NULL;

    private:
        void PaintBackground(HDC hdc, RECT const & rectClient) const;
        void PaintBorder(HDC hdc, RECT const & rectClient, Edge const edge) const;
    };
}