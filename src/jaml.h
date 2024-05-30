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

    enum Edge : uint8_t
    {
        TOP = 0,
        LEFT = 1,
        BOTTOM = 2,
        RIGHT = 3,
        CENTER = 4, // only for text align
        AUTO = (std::numeric_limits<uint8_t>::max)()
    };
    Edge operator ~(Edge const edge);

    enum Side : uint8_t
    {
        INNER = 0, OUTER = 1
    };
    Side operator ~(Side const side);

    enum Dimension : uint8_t
    {
        WIDTH = 0, HEIGHT = 1
    };
    
    Dimension edgeToDimension(Edge const edge);

    enum Unit
    {
        PX, EM, PT, PC, NONE = (std::numeric_limits<uint8_t>::max)()
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

        std::optional<int> toPixels(Element * context, Dimension const dim, Side const side = OUTER) const;
    };


    class Tether
    {
    public:
        Tether() : edge(Edge::AUTO) {};
        Tether(std::string_view const & id, Edge const edge, Measure const & offset) : id(id), edge(edge), offset(offset) {};
        std::string id;
        Edge edge;
        Measure offset;
    };

    class ResolvedPos
    {
    public:
        std::optional<int> coord[4] = { std::nullopt };
        std::optional<int> size[2] = { std::nullopt };
    };

    class Color
    {
    public:
        Color() {};
        Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) {};
        uint8_t red() const noexcept { return r; };
        uint8_t green() const noexcept { return g; };
        uint8_t blue() const noexcept { return b; };
        uint8_t alpha() const noexcept { return a; };
        void red(uint8_t v) noexcept { r = v; };
        void green(uint8_t v) noexcept { g = v; };
        void blue(uint8_t v) noexcept { b = v; };
        void alpha(uint8_t v) noexcept { a = v; };
        COLORREF ref() const noexcept { return RGB(r, g, b); }
        static Color parse(std::string const & spec);
    private:
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;
    };

    class Element
    {
    public:
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
        void setType(ElementType const v);
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

        Resolved recalculatePos(Side const side, Edge const edge, bool * canMakeStuffUp = nullptr);
        Resolved recalculateDimension(Side const side, Dimension const dim);

        // Resolves as many coordinates as possible (single pass) and returns the number of unresolved coordinates/dimensions.
        size_t recalculateLayout(bool * canMakeStuffUp = nullptr);

        // Applies a tether offset to an otherwise resolved coordinate. Returns 1 if offset is unresolved (and resets the coord), 0 otherwise.
        Resolved applyOffset(Side const side, Edge const edge, Measure const & offset, bool * canMakeStuffUp = nullptr);

        // Move futurePos to currentPos and redraw everything
        void commitLayout();

        ResolvedPos futurePos[2];

    private:
        Element(std::string_view const & source);
        void create();

    protected:
        ResolvedPos currentPos[2];
        HBRUSH backgroundBrush = NULL;
        Color backgroundColor = { 0xFF, 0xFF, 0xFF, 0xFF };
        std::vector<std::shared_ptr<Element>> children = {};
        std::vector<std::string> classes;
        bool created = false;
        std::string fontFace;
        FontStyle fontStyle = FontStyle::INHERIT;
        int fontWeight = 0;
        Measure fontSize = {0, Unit::NONE};
        HFONT font = 0;
        Edge textAlignH = LEFT;
        HWND hwndInner = 0;
        HWND hwndOuter = 0;
        size_t i = 0;
        std::string id;
        HBITMAP image = 0;
        std::string label;
        uint8_t opacity = 255;
        Measure padding[4];
        Element * parent = nullptr;
        Measure size[2] = {{0, NONE}, {0, NONE}};
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
    int getFontHeight(HWND hwnd);
    int getLineHeight(HWND hwnd);
    int getDpi(HWND hwnd);

    bool isHEdge(Edge const side);
    bool isVEdge(Edge const side);

    // Internal logging function
    void jaml_log(JamlLogSeverity const & sev, char const * message);

    std::string edgeToString(Edge const edge);
}