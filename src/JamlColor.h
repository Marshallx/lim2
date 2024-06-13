#pragma once

#include <unordered_map>
#include <string>

#include <Windows.h>

namespace jaml
{

    class Color
    {
    public:
        Color() {};
        Color(uint32_t rgb) : r(GetRValue(rgb)), g(GetGValue(rgb)), b(GetBValue(rgb)) {};
        Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) {};
        uint8_t red() const noexcept { return r; };
        uint8_t green() const noexcept { return g; };
        uint8_t blue() const noexcept { return b; };
        uint8_t alpha() const noexcept { return a; };
        void red(uint8_t v) noexcept { r = v; };
        void green(uint8_t v) noexcept { g = v; };
        void blue(uint8_t v) noexcept { b = v; };
        void alpha(uint8_t v) noexcept { a = v; };
        uint32_t rgb() const noexcept { return RGB(r, g, b); }
        void rgb(uint32_t rgb) noexcept { r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb); }
        uint32_t argb() const noexcept { return RGB(r, g, b) | a<<24; }
        void argb(uint32_t argb) noexcept { r = GetRValue(argb); g = GetGValue(argb); b = GetBValue(argb); a = HIBYTE(argb >> 16); }
        static Color Parse(std::string_view const & spec);

    private:
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;
    };
}