#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <Windows.h>

namespace jaml
{
    // ========================================================================
    // ================ Forward declarations ==================================
    // ========================================================================

    class Window;
    class Element;

    // ========================================================================
    // ================ Enums =================================================
    // ========================================================================

    enum Axis
    {
        X, Y
    };

    enum ElementType
    {
        STATIC, BUTTON, COMBOBOX, EDIT, LISTBOX, CHECKBOX
    };

    enum FontStyle
    {
        INHERIT = 0,
        NORMAL = 1 << 0,
        ITALIC = 1 << 1,
        UNDERLINE = 1 << 2,
        STRIKETHROUGH = 1 << 4
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

    enum JamlLogSeverity
    {
        SEV_INFO, SEV_WARN, SEV_ERROR
    };

    enum Resolved
    {
        RESOLVED = 0, UNRESOLVED = 1
    };

    enum Side
    {
        TOP, LEFT, BOTTOM, RIGHT, CENTER /*only for text align*/, NONE
    };

    Side operator ~(Side const side);

    enum Unit
    {
        PX, EM, PT, AUTO // PC
    };

    // ========================================================================
    // ================ Classes ===============================================
    // ========================================================================

    class Measure
    {
    public:
        double value;
        Unit unit;

        Measure(double const value, Unit const unit) : unit(unit), value(value) {};
        Measure(double const value) : value(value), unit(PX) {};
        Measure() : value(0), unit(PX) {};

        int toPixels(Element const * element) const;
    };


    class Tether
    {
    public:
        Tether() : side(NONE) {};
        Tether(std::string_view const & id, Side const side, Measure const & offset) : id(id), side(side), offset(offset) {};
        std::string id;
        Side side;
        Measure offset;
    };

    class Size
    {
    public:
        Measure width;
        Measure height;
    };

    class ResolvedPos
    {
    public:
        std::optional<int> coord[4] = { std::nullopt };
        std::optional<int> width = std::nullopt;
        std::optional<int> height = std::nullopt;
    };

    class Color
    {
    public:
        Color() {};
        Color(uint32_t value) : rgba(value) {};
        Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : rgba(RGB(red, green, blue) | (((DWORD)(BYTE)(alpha))<<24)) {};
        uint32_t value() const noexcept { return rgba; }
        uint8_t red() const noexcept { return LOBYTE(rgba); };
        uint8_t green() const noexcept { return (LOBYTE(((WORD)(rgba)) >> 8)); };
        uint8_t blue() const noexcept { return LOBYTE(rgba); };
        uint8_t alpha() const noexcept { return rgba >> 24; };
        void value(uint32_t value) noexcept { rgba = value; }
        void red(uint8_t value) noexcept { rgba = (rgba & 0xFFFFFF00) | value; };
        void green(uint8_t value) noexcept { rgba = (rgba & 0xFFFF00FF) | (value << 8); };
        void blue(uint8_t value) noexcept { rgba = (rgba & 0xFF00FFFF) | (value << 16); };
        void alpha(uint8_t value) noexcept { rgba = (rgba & 0x00FFFFFF) | (value << 24); };
    private:
        uint32_t rgba = 0;
    };

    class Element
    {
    public:
        Element * addChild(std::string_view const & id = {});

        Element * getChild(size_t const i) const;
        Element * findElement(std::string_view const & id);
        Window * getRoot() const noexcept;

        void removeChildren();
        void remove();

        Color getBackgroundColor() const noexcept;
        HWND getHwnd() const noexcept;
        HFONT getFont() const;
        std::string const & getFontFace() const;
        Measure const & getFontSize() const;
        FontStyle const & getFontStyle() const;
        int getFontWeight() const;
        Color getTextColor() const noexcept;
        std::string const & getValue() const;

        static Measure parseMeasure(std::string const & spec);

        void setBackgroundColor(Color const & v);
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
        void setPadding(Side const side, Measure const & v);
        void setTextAlignH(Side const v);
        void setTextColor(Color const & v);
        void setType(ElementType const v);
        void setType(std::string_view const & v);
        void setValue(std::string_view const & v);
        void setVisible(bool const v = true);
        void setWidth(std::string const & spec);

        void tether(Side const mySide, std::string_view const & otherId, Side const otherSide, Measure const & offset);
        void tether(Side const mySide, std::string const & spec);

        void updateFont();

        void show();
        void hide();

        // Resolves the specified coordinate, if possible. Returns 1 if coord is unresolved, 0 otherwise.
        Resolved recalculatePos(Side const side, bool * canMakeStuffUp = nullptr);

        // Resolves the width. Returns 1 if width is unresolved, 0 otherwise.
        Resolved recalculateWidth();

        // Resolves the height. Returns 1 if height is unresolved, 0 otherwise.
        Resolved recalculateHeight();

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t recalculateLayout(bool * canMakeStuffUp = nullptr);

        // Applies a tether offset to an otherwise resolved coordinate. Returns 1 if offset is unresolved (and resets the coord), 0 otherwise.
        Resolved applyOffset(Side const side, Measure const & offset, bool * canMakeStuffUp = nullptr);

        // Move futurePos to currentPos and redraw everything
        void commitLayout();

    private:
        Element(std::string_view const & source);
        void create();

    protected:
        ResolvedPos currentPos;
        ResolvedPos futurePos;
        Color backgroundColor = { 0xFFFFFFFF };
        std::vector<std::shared_ptr<Element>> children = {};
        std::vector<std::string> classes;
        bool created = false;
        std::string fontFace;
        FontStyle fontStyle = FontStyle::INHERIT;
        int fontWeight = 0;
        Measure fontSize = {0, Unit::AUTO};
        HFONT font = 0;
        Side textAlignH = LEFT;
        HWND hwnd = 0;
        size_t i = 0;
        std::string id;
        HBITMAP image = 0;
        std::string label;
        uint8_t opacity = 255;
        Measure padding[4];
        Element * parent = nullptr;
        Size size = { { 0, AUTO }, { 0, AUTO } };
        Tether tethers[4];
        Color textColor;
        ElementType type = ElementType::STATIC;
        std::string value;
        bool visible = true;

    protected:
        Element() {};
    };

    class JamlParser
    {
    public:
        JamlParser(std::string_view const & source, Window * window);

    private:
        std::string_view source;
        size_t pos = 0;
        size_t line = 1;
        size_t col = 1;
        Element * elem = nullptr;

        void eat_whitespace();
        std::string_view parse_key();
        std::string parse_val();
        void parse_node();

    };

    class Window : public Element
    {
    public:
        Window();
        Window(std::filesystem::path const & file);
        Window(std::string_view const & source);
        int start(HINSTANCE hInstance, int const nCmdShow);
        void setForceResolve(bool const force = true);

    private:
        void defaultFont();
        Window(Window const &) = delete;
        bool throwOnUnresolved = true;
    };

    // ========================================================================
    // ================ Functions =============================================
    // ========================================================================

    HFONT createFont(HWND const hwnd, std::string const & fontFace, int const fontSize, FontStyle fontStyle = FontStyle::NORMAL, int fontWeight = FontWeight::REGULAR);
    int getFontHeight(HWND hwnd, HFONT font);
    int getLineHeight(HWND hwnd, HFONT font);
    int getDpi(HWND hwnd);

    bool isHSide(Side const side);
    bool isVSide(Side const side);

    // Internal logging function
    void jaml_log(JamlLogSeverity const & sev, char const * message);

    std::string sideToString(Side const side);
}