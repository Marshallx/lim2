#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <Windows.h>

#include "JamlElement.h"

namespace jaml
{
    enum Axis
    {
        X = 0, Y = 1
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

    class Measure
    {
    public:
        static Measure Parse(std::string_view const & spec);
        double value;
        Unit unit;

        Measure(double const value, Unit const unit) : unit(unit), value(value) {};
        Measure(double const value) : value(value), unit(PX) {};
        Measure() : value(0), unit(PX) {};

        std::optional<int> toPixels(JamlElement const * context, Dimension const dim, Side const side = OUTER) const;
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

    int getDpi(HWND hwnd);
    bool isHEdge(Edge const side);
    bool isVEdge(Edge const side);
}