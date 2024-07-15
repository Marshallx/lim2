#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "MxiUtils.h"

namespace jass
{
    static constexpr auto const kBorder = "border";
    static constexpr auto const kBorderBottom = "border-bottom";
    static constexpr auto const kBorderLeft = "border-left";
    static constexpr auto const kBorderRight = "border-right";
    static constexpr auto const kBorderTop = "border-top";
    static constexpr auto const kBorderWidth = "border-width";
    static constexpr auto const kBottom = "bottom";
    static constexpr auto const kHeight = "height";
    static constexpr auto const kLeft = "left";
    static constexpr auto const kMargin = "margin";
    static constexpr auto const kMarginBottom = "margin-bottom";
    static constexpr auto const kMarginLeft = "margin-left";
    static constexpr auto const kMarginRight = "margin-right";
    static constexpr auto const kMarginTop = "margin-top";
    static constexpr auto const kPadding = "padding";
    static constexpr auto const kPaddingBottom = "padding-bottom";
    static constexpr auto const kPaddingLeft = "padding-left";
    static constexpr auto const kPaddingRight = "padding-right";
    static constexpr auto const kPaddingTop = "padding-top";
    static constexpr auto const kPosition = "position";
    static constexpr auto const kRight = "right";
    static constexpr auto const kTop = "top";
    static constexpr auto const kWidth = "width";

    MX_COLLECTION(Dimension, WIDTH, HEIGHT);};

    MX_COLLECTION(Edge, TOP, LEFT, BOTTOM, RIGHT, ALL_EDGES);
    Edge const & operator ~()
    {
        switch (value)
        {
        case Value::TOP: return Edge::BOTTOM;
        case Value::LEFT: return Edge::RIGHT;
        case Value::BOTTOM: return Edge::TOP;
        case Value::RIGHT: return Edge::LEFT;
        }
        return Edge::ALL_EDGES;
    };
    operator Dimension const &()
    {
        if (value == Value::TOP || value == Value::BOTTOM) return Dimension::HEIGHT;
        return Dimension::WIDTH;
    };
    };

    char const * const GetBorderProp(Edge const & edge)
    {
        switch (edge)
        {
        case Edge::Value::TOP: return kBorderTop;
        case Edge::Value::LEFT: return kBorderLeft;
        case Edge::Value::BOTTOM: return kBorderBottom;
        case Edge::Value::RIGHT: return kBorderRight;
        }
    }

    char const * const GetEdgeProp(Edge const & edge)
    {
        switch (edge)
        {
        case Edge::Value::TOP: return kTop;
        case Edge::Value::LEFT: return kLeft;
        case Edge::Value::BOTTOM: return kBottom;
        case Edge::Value::RIGHT: return kRight;
        }
    }

    char const * const GetMarginProp(Edge const & edge)
    {
        switch (edge)
        {
        case Edge::Value::TOP: return kMarginTop;
        case Edge::Value::LEFT: return kMarginLeft;
        case Edge::Value::BOTTOM: return kMarginBottom;
        case Edge::Value::RIGHT: return kMarginRight;
        }
    }

    char const * const GetPaddingProp(Edge const & edge)
    {
        switch (edge)
        {
        case Edge::Value::TOP: return kPaddingTop;
        case Edge::Value::LEFT: return kPaddingLeft;
        case Edge::Value::BOTTOM: return kPaddingBottom;
        case Edge::Value::RIGHT: return kPaddingRight;
        }
    }

    char const * const GetSizeProp(Dimension const & dim)
    {
        switch (dim)
        {
        case Dimension::Value::WIDTH: return kWidth;
        case Dimension::Value::HEIGHT: return kHeight;
        }
    }

    class Property
    {
    public:
        Property(std::string_view const & prop, std::string_view const & value, size_t line, size_t col);
        std::string m_prop;
        std::string m_value;
        bool m_important;
        size_t m_line;
        size_t m_col;
    };

    enum Combinator
    {
        NONE, DESCENDANT, CHILD, SUBSEQUENT_SIBLING, NEXT_SIBLING, COLUMN
    };

    enum SimpleSelectorType
    {
        NONE, TYPE, ID, CLASS, ATTRIBUTE, PSEUDO_CLASS, PSEUDO_ELEMENT
    };

    class Selector
    {
    public:
        std::string type = {};
        std::string id = {};
        std::vector<std::string> classes = {};
        std::vector<std::string> attributes = {};
        std::vector<std::string> pseudoclasses = {};
        Combinator combinator = Combinator::NONE; // relation to parent (left) CompoundSelector
    };

    class Rule
    {
    public:
        Rule(size_t line, size_t col) : m_line(line), m_col(col) {}
        std::vector<std::pair<uint64_t, std::vector<Selector>>> selectors = {};
        std::unordered_map<char const *, Property> styles = {};
        size_t m_line;
        size_t m_col;
    };

    class JassParser
    {
    public:
        JassParser(std::string_view const & source, std::vector<Rule> & rules);

    private:
        std::string_view const & source;
        char c = 0;
        size_t size;
        size_t pos = 0;
        size_t col = 0;
        size_t line = 0;

        [[noreturn]] void Error(std::string_view const & msg) const;
        void Expect(char const expected) const;
        void Eat(std::string_view const & expected);
        void RememberPos(bool const stash = true);
        bool LookAhead(std::string_view const & expected, bool const eatIfFound = false);
        void EatWhitespace();
        void EatCommentsAndWhitespace();
        char NextChar();
        Rule ParseRule();
        std::vector<std::string> ParseSelectors();
        std::string_view ParseKey();
        std::string_view ParseValue();
        const char * ValidateProperty(std::string_view const & k) const;
    };

}