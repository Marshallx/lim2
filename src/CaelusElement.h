#pragma once

#include <Windows.h>

#include "jass.h"

#include "CaelusClass.h"
#include "MxiLogging.h"
#include "MxiUtils.h"

namespace Caelus
{
    using namespace jass;

    constexpr static auto const kElementClass = L"Caelus_ELEMENT";
    LRESULT CaelusElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    class CaelusWindow;

    class CaelusElement
    {
        friend class CaelusWindow;
        friend class JamlParser;
    public:
        // Painting
        static void Register(HINSTANCE hInstance, wchar_t const * standardClass = nullptr, wchar_t const * caelusClass = nullptr, CaelusElementType const type = GENERIC);
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
                case BORDER_WIDTH:
                case LABEL:
                case PADDING:
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

            // Default styles
            if constexpr (std::is_same_v<T, Color>)
            {
                switch (style)
                {
                case BACKGROUND_COLOR: { static std::optional<Color> v = 0xFFFFFF; return v; }
                case BORDER_COLOR: { static std::optional<Color> v = 0; return v; }
                case TEXT_COLOR: { static std::optional<Color> v = 0; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, Measure>)
            {
                switch (style)
                {
                case BORDER_RADIUS: { static std::optional<Measure> v = { Measure::Parse("0") }; return v; }
                case BORDER_WIDTH: { static std::optional<Measure> v = { Measure::Parse("0") }; return v; }
                case FONT_SIZE: { static std::optional<Measure> v = { Measure::Parse("12pt") }; return v; }
                case PADDING: { static std::optional<Measure> v = { Measure::Parse("0") }; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                switch (style)
                {
                case FONT_FACE: { static std::optional<std::string> v = "Arial"; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                switch (style)
                {
                case FONT_ITALIC: { static std::optional<bool> v = false; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, int>)
            {
                switch (style)
                {
                case FONT_WEIGHT: { static std::optional<int> v = FontWeight::REGULAR; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, Tether>)
            {
            }
            else if constexpr (std::is_same_v<T, Edge>)
            {
                switch (style)
                {
                case TEXT_ALIGNH: { static std::optional<Edge> v = Edge::LEFT; return v; }
                case TEXT_ALIGNV: { static std::optional<Edge> v = Edge::TOP; return v; }
                }
            }
            else if constexpr (std::is_same_v<T, CaelusElementType>)
            {
                switch (style)
                {
                case ELEMENT_TYPE: { static std::optional<CaelusElementType> v = CaelusElementType::GENERIC; return v; }
                }
            }
            else
            {
                static_assert(mxi::always_false<T>, "Unsupported type for GetStyle()");
            }

            static std::optional<T> def{};
            return def;
        }

        CaelusElement const * find(size_t uid) const;
        CaelusElement const * find(std::string_view const & search) const;

        Color const & GetBackgroundColor() const { return GetStyle<Color>(BACKGROUND_COLOR).value(); }
        Color const & GetBorderColor(Edge const edge) const { return GetStyle<Color>(BORDER_COLOR, edge).value(); }
        Color const & GetTextColor() const { return GetStyle<Color>(TEXT_COLOR).value(); }
        Measure const & GetBorderRadius(Corner const corner) const { return GetStyle<Measure>(BORDER_RADIUS, corner).value(); }
        Measure const & GetBorderWidth(Edge const edge) const { return GetStyle<Measure>(BORDER_WIDTH, edge).value(); }
        Measure const & GetFontSize() const { return GetStyle<Measure>(FONT_SIZE).value(); }
        CaelusElementType GetElementType() const { auto const & type = GetStyle<CaelusElementType>(ELEMENT_TYPE); return type.has_value() ? type.value() : GENERIC; }
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
        void SetElementType(std::string_view const & type);
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
        HDWP CommitLayout(HINSTANCE hInstance, HDWP hdwp, HWND outerWindow = NULL);

        CaelusElement * GetSibling(std::string_view const & name) const;
        CaelusElement * GetSibling(Edge const edge) const;

        std::vector<CaelusElement> m_children = {};
        CaelusClass * m_class = nullptr;
        std::string m_name;
        CaelusElement * m_parent = nullptr;
        ResolvedRect m_currentRect;
        ResolvedRect m_futureRect;
        HFONT m_hfont = NULL;
        HWND m_hwnd = 0;


    private:
        std::string const & GetCssProp(char const * property) const;
        bool SelectedBy(jass::Rule const & rule) const;
        std::unordered_map<std::string, std::string> m_attributes = {};
        std::unordered_map<char const *, std::string> m_styles = {};
        std::string m_tagname = {};
        std::string m_text = {};

        void PaintBackground(HDC hdc, RECT const & rectClient) const;
        void PaintBorder(HDC hdc, RECT const & rectClient, Edge const edge) const;
        LRESULT CallStandardWndProc();
        static WNDPROC StandardWndProc[CaelusElementType::last];
        static wchar_t const * CaelusClassName[CaelusElementType::last];
    };
}