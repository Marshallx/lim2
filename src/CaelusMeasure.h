#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <Windows.h>

namespace Caelus
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
        ALL_EDGES = 4
    };
    Edge operator ~(Edge const edge);

    enum Corner : uint8_t
    {
        TOPLEFT = 0,
        TOPRIGHT = 1,
        BOTTOMLEFT = 2,
        BOTTOMRIGHT = 3,
        ALL_CORNERS = 4
    };

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
        Measure(Measure const &) = default;
    };

    class Tether
    {
    public:
        Tether(std::string_view const & id, Edge const edge, Measure const & offset) : id(id), edge(edge), offset(offset) {};
        std::string id;
        Edge edge;
        Measure offset;
    };

    class ResolvedRect
    {
    public:
        int GetBorder(Edge const edge) const;
        int GetEdge(Edge const edge) const;
        int GetSize(Dimension const dim) const;
        int GetPadding(Edge const edge) const;
        int GetNC(Edge const edge) const;

        bool HasBorder(Edge const edge) const;
        bool HasEdge(Edge const edge) const;
        bool HasSize(Dimension const dim) const;
        bool HasPadding(Edge const edge) const;
        bool HasNC(Edge const edge) const;

        void SetBorder(Edge const edge, int px);
        void SetEdge(Edge const edge, int px);
        void SetSize(Dimension const dim, int px);
        void SetInnerSize(Dimension const dim, int px);
        void SetPadding(Edge const edge, int px);

        size_t CountUnresolved() const;
    private:
        std::optional<int> m_edge[4];
        std::optional<int> m_padd[4];
        std::optional<int> m_bord[4];
        std::optional<int> m_size[2];
        std::optional<int> m_innerSize[2];
        void SetBottom(int px);
        void SetHeight(int px);
        void SetInnerHeight(int px);
        void SetInnerWidth(int px);
        void SetLeft(int px);
        void SetRight(int px);
        void SetTop(int px);
        void SetWidth(int px);
    };

    std::string edgeToKeyword(Edge const edge);
    Edge keywordToEdge(std::string_view const & keyword);
    int getDpi(HWND hwnd);
    int getFontHeight(HWND hwnd);
    int getLineHeight(HWND hwnd);
    int getLineHeight(HFONT hfont);
    bool isHEdge(Edge const side);
    bool isVEdge(Edge const side);
    bool isFarEdge(Edge const edge);
}