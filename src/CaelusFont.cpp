#include "CaelusFont.h"

namespace Caelus
{
    int getFontHeight(HWND hwnd)
    {
        TEXTMETRIC tm = {};
        auto const hdc = GetDC(hwnd);
        auto const r = GetTextMetrics(hdc, &tm);
        ReleaseDC(hwnd, hdc);
        return tm.tmHeight;
    }

    int getLineHeight(HWND hwnd)
    {
        OUTLINETEXTMETRIC tm = {};
        auto const hdc = GetDC(hwnd);
        auto const r = GetOutlineTextMetrics(hdc, sizeof(OUTLINETEXTMETRIC), &tm);
        ReleaseDC(hwnd, hdc);
        auto lineHeight = (-tm.otmDescent) + tm.otmLineGap + tm.otmAscent;
        return MulDiv(lineHeight, getDpi(hwnd), 96);
    }

    void Font::SetColor(std::string_view const & color)
    {
        m_color = Color::Parse(color);
    }

    void Font::SetColor(Color const & color)
    {
        m_color = color;
    }

    void Font::SetColor(uint32_t const color)
    {
        m_color = { color };
    }

    void Font::SetFace(std::string_view const & face)
    {
        m_face = face;
    }

    void Font::SetSize(std::string_view const & size)
    {
        m_size = Measure::Parse(size);
    }

    void Font::SetStyle(std::string_view const & style)
    {
        if (style == "italic") m_italic = true;
        else if (style == "inherit") m_italic.reset();
        else m_italic = false;
    }

    void Font::SetWeight(std::string_view const & weight)
    {
        if (weight == "inherit") m_weight.reset();
        else if (weight == "thin") m_weight = THIN;
        else if (weight == "extralight") m_weight = EXTRALIGHT;
        else if (weight == "light") m_weight = LIGHT;
        else if (weight == "regular") m_weight = REGULAR;
        else if (weight == "medium") m_weight = MEDIUM;
        else if (weight == "semibold") m_weight = SEMIBOLD;
        else if (weight == "bold") m_weight = BOLD;
        else if (weight == "extrabold") m_weight = EXTRABOLD;
        else if (weight == "black") m_weight = BLACK;
        else SetWeight(atoi(std::string(weight).c_str()));
    }

    void Font::SetWeight(int const weight)
    {
        m_weight = (weight <= 0) ? REGULAR : weight;
    }
}