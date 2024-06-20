#pragma once

#include <optional>
#include <string>

#include "CaelusColor.h"
#include "CaelusMeasure.h"

namespace Caelus
{
    enum FontWeight : int
    {
        INHERIT = 0,
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

    class Font
    {
    private:
        std::optional<Color> m_color;
        std::optional<std::string> m_face;
        std::optional<bool> m_italic;
        std::optional<Measure> m_size;
        std::optional<int> m_weight;

    public:
        void SetColor(std::string_view const & color);
        void SetColor(Color const & color);
        void SetColor(uint32_t const color);
        void SetFace(std::string_view const & face);
        void SetSize(std::string_view const & size);
        void SetStyle(std::string_view const & style);
        void SetWeight(std::string_view const & weight);
        void SetWeight(int const weight);

        auto const & GetColor() const noexcept { return m_color; }
        auto const & GetFace() const noexcept { return m_face; };
        auto const & GetItalic() const noexcept { return m_italic; };
        auto const & GetSize() const noexcept { return m_size; };
        auto const & GetWeight() const noexcept { return m_weight; };
    };

    int getFontHeight(HWND hwnd);
    int getLineHeight(HWND hwnd);
}