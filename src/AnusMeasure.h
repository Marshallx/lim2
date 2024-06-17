#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <Windows.h>

#include "AnusElement.h"

namespace Anus
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
        CENTER = 4 // only for text align
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
        PX, EM, PT, PC
    };

    enum Resolved
    {
        RESOLVED = 0, UNRESOLVED = 1
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

        std::optional<int> toPixels(AnusElement const * context, Dimension const dim, Side const side = OUTER) const;
    };

    class Tether
    {
    public:
        Tether() {};
        Tether(std::string_view const & id, Edge const edge, Measure const & offset) : id(id), edge(edge), offset(offset) {};
        std::string id;
        Edge edge;
        Measure offset;
    };

    class ResolvedRect
    {
    public:
        int GetEdge(Edge const edge) const;
        int GetSize(Dimension const dim) const;
        bool HasEdge(Edge const edge) const;
        bool HasSize(Dimension const dim) const;
        void SetEdge(Edge const edge, int px);
        void SetSize(Dimension const dim, int px);
        size_t CountUnresolved() const;
    private:
        std::optional<int> m_edge[4];
        std::optional<int> m_size[2];
        void SetTop(int px);
        void SetLeft(int px);
        void SetBottom(int px);
        void SetRight(int px);
        void SetWidth(int px);
        void SetHeight(int px);
    };

    std::string edgeToKeyword(Edge const edge);
    Edge edgeFromKeyword(std::string_view const & keyword);
    int getDpi(HWND hwnd);
    bool isHEdge(Edge const side);
    bool isVEdge(Edge const side);
    bool isFarEdge(Edge const edge);
}