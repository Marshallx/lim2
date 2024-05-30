#include <deque>
#include <iostream>
#include <fstream>
#include <limits>
#include <regex>

#include <Windows.h>
#include <CommCtrl.h>

#include "..\resource\Resource.h"

#include "MxUtils.h"

#include "jaml.h"

namespace jaml
{
    using namespace mx::MxUtils;

    // ========================================================================
    // ================ Globals ===============================================
    // ========================================================================

    HINSTANCE g_hInstance;

    // ========================================================================
    // ================ Functions =============================================
    // ========================================================================

    int du2px(HWND const hwnd, int const du, Axis axis)
    {
        RECT r = {};
        r.left = r.top = du;
        r.right = r.bottom = 0;
        MapDialogRect(hwnd, &r);
        return axis == X ? r.left : r.top;
    }

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

    int getDpi(HWND hwnd)
    {
        HDC hdc = GetDC(hwnd);
        if (!hdc) return 0;
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(0, hdc);
        return dpi;
    }

    bool isHEdge(Edge const edge)
    {
        return edge == LEFT || edge == RIGHT;
    }

    bool isVEdge(Edge const edge)
    {
        return edge == TOP || edge == BOTTOM;
    }

    std::string edgeToString(Edge const edge)
    {
        switch (edge)
        {
        case TOP: return "top";
        case LEFT: return "left";
        case BOTTOM: return "bottom";
        case RIGHT: return "right";
        case CENTER: return "center";
        case AUTO: return "auto";
        }
        MX_THROW("Unknown edge.")
    }

    void jaml_log(JamlLogSeverity const & sev, char const * message)
    {
        std::cout << "JAML LOG: " << message;
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_COMMAND: // Application menu
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CTLCOLORSTATIC:
        {
            auto elem = (Element *)GetPropA((HWND)lParam, "elem");
            if (!elem) return DefWindowProc(hWnd, message, wParam, lParam);
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, elem->getTextColor().ref());
            SetBkColor(hdcStatic, elem->getBackgroundColor().ref());
            return (INT_PTR)elem->getBackgroundBrush();
        }
        break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    // Message handler for about box.
    INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);
        switch (message)
        {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
        return (INT_PTR)FALSE;
    }

    // ========================================================================
    // ================ Methods ===============================================
    // ========================================================================

    Color Color::parse(std::string const & spec)
    {
        constexpr static auto const pattern = R"(^\s*(?:(#|0x)\s*([0-9a-f]{2})\s*,?\s*([0-9a-f]{2})\s*?\s*([0-9a-f]{2}))|(rgb)\s*(?:\(|\s)?\s*([0-9]*(?:\.[0-9]*)%?)\s*[,\s]\s*([0-9]*(?:\.[0-9]*)%?)\s*[,\s]\s*([0-9]*(?:\.[0-9]*)%?)\s*\)?\s*$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript | std::regex_constants::icase);

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW(std::format("Invalid color: bad format. Expected #RRGGBB or rgb(rrr,ggg,bbb) or rgb(N.F[%],N.F[%],N.F[%]), saw {}", spec).c_str());

        auto const type = matches[1].str();
        auto const red = matches[2].str();
        auto const green = matches[3].str();
        auto const blue = matches[4].str();

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        constexpr static auto const parseComponent = [](std::string const & component)
        {
            size_t r = 0;
            if (component.find_first_of('%') != std::string::npos)
            {
                r = (size_t)((std::stod(component) / 100.0L) * 255.0L);
            }
            else if (component.find_first_of('.') != std::string::npos)
            {
                r = (size_t)(std::stod(component) * 255.0L);
            }
            else
            {
                r = (size_t)std::stoull(component);
            }
            if (r > 255) MX_THROW(std::format("Invalid color: each component must resolve to an integer between 0 and 255. Saw \"{}\" => {}", component, r).c_str());
            return (uint8_t)r;
        };

        if (type == "#" || type == "0x" || type == "0X")
        {
            r = (uint8_t)std::stoi(red, nullptr, 16);
            g = (uint8_t)std::stoi(green, nullptr, 16);
            b = (uint8_t)std::stoi(blue, nullptr, 16);
        }
        else
        {
            r = parseComponent(red);
            g = parseComponent(green);
            b = parseComponent(blue);
        }
        return { r,g,b };
    }

    Element * Element::addChild(std::string_view const & id)
    {
        auto const child = std::make_shared<Element>(Element{});
        child.get()->id = id.empty() ? create_guid() : id;
        child.get()->parent = this;
        child.get()->i = children.size();
        children.push_back(child);
        return child.get();
    }

    Resolved Element::applyOffset(Side const side, Edge const edge, Measure const & offset, bool * canMakeStuffUp)
    {
        switch (offset.unit)
        {
        case PX:
            futurePos[side].coord[edge] = futurePos[side].coord[edge].value() + static_cast<int>(offset.value);
            return RESOLVED;

        case EM:
            futurePos[side].coord[edge] = futurePos[side].coord[edge].value() + static_cast<int>(
                static_cast<double>(getFontHeight(hwndInner)) * offset.value);
            return RESOLVED;

        case PC:
            auto const of = getParent()->futurePos[side].size[edgeToDimension(edge)];
            if (of.has_value())
            {
                futurePos[side].coord[edge] = futurePos[side].coord[edge].value() + static_cast<int>(
                    static_cast<double>(of.value()) * offset.value);
                return RESOLVED;
            }
            break;
        }

        MX_THROW("Unsupported unit for offset calculation.");

        futurePos[side].coord[edge].reset();
        return UNRESOLVED;
    }

    void Element::commitLayout()
    {
        currentPos[OUTER] = futurePos[OUTER]; futurePos[OUTER] = {};
        currentPos[INNER] = futurePos[INNER]; futurePos[INNER] = {};

        if (!hwndOuter)
        {
            create();
        }
        else
        {
            // Reposition everything
            SetWindowPos(hwndOuter, 0,
                currentPos[OUTER].coord[LEFT].value(),
                currentPos[OUTER].coord[TOP].value(),
                currentPos[OUTER].size[WIDTH].value(),
                currentPos[OUTER].size[HEIGHT].value(),
                SWP_NOZORDER
            );
            SetWindowPos(hwndOuter, 0,
                currentPos[INNER].coord[LEFT].value(),
                currentPos[INNER].coord[TOP].value(),
                currentPos[INNER].size[WIDTH].value(),
                currentPos[INNER].size[HEIGHT].value(),
                SWP_NOZORDER
            );
        }


        for (auto child : children)
        {
            child.get()->commitLayout();
        }
    }

    void Element::create()
    {
        if (hwndOuter) throw std::runtime_error("Element window already created!");
        auto * winClassName = L"STATIC";
        DWORD style = WS_CHILD | WS_VISIBLE;

        switch (type)
        {
        case ElementType::BUTTON:
            winClassName = L"BUTTON";
            style |= WS_TABSTOP | WS_BORDER;
            break;

        case ElementType::COMBOBOX:
            winClassName = L"COMBOBOX";
            style |= WS_TABSTOP;
            break;

        case ElementType::EDIT:
            winClassName = L"EDIT";
            style |= WS_TABSTOP;
            break;

        case ElementType::LISTBOX:
            winClassName = L"LISTBOX";
            style |= WS_TABSTOP;
            break;

        case ElementType::STATIC:
            winClassName = L"STATIC";
            break;

        default:
            throw std::runtime_error("Unknown ElementType.");
        }

        switch (textAlignH)
        {
        case LEFT: style |= ES_LEFT; break;
        case CENTER: style |= ES_CENTER; break;
        case RIGHT: style |= ES_RIGHT; break;
        }

        hwndOuter = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, currentPos[OUTER].coord[LEFT].value(), currentPos[OUTER].coord[TOP].value(), currentPos[OUTER].size[WIDTH].value(), currentPos[OUTER].size[HEIGHT].value(), parent->hwndInner, NULL, g_hInstance, NULL);
        if (!hwndOuter) MX_THROW("Failed to create element outer window");
        hwndInner = CreateWindow(winClassName, Utf16String(label).c_str(), style, currentPos[INNER].coord[LEFT].value(), currentPos[INNER].coord[TOP].value(), currentPos[INNER].size[WIDTH].value(), currentPos[INNER].size[HEIGHT].value(), hwndOuter, NULL, g_hInstance, NULL);
        if (!hwndInner) MX_THROW("Failed to create element inner window");
        
        SetPropA(hwndOuter, "elem", this);
        SetPropA(hwndInner, "elem", this);
        updateFont();
        SendMessage(hwndInner, WM_SETFONT, (WPARAM)getFont(), NULL);
        SendMessage(hwndOuter, WM_SETFONT, (WPARAM)getFont(), NULL);
    }

    Element * Element::findElement(std::string_view const & id)
    {
        if (this->id == id) return this;
        for (auto const & child : children)
        {
            auto const found = child.get()->findElement(id);
            if (found) return found;
        }
        return nullptr;
    }

    HBRUSH Element::getBackgroundBrush() const noexcept
    {
        return backgroundBrush;
    }

    Color const & Element::getBackgroundColor() const noexcept
    {
        return backgroundColor;
    }

    Element * Element::getChild(size_t const i) const
    {
        return children.at(i).get();
    }

    HFONT Element::getFont() const
    {
        auto element = this;
        while (!element->font) element = element->parent;
        return element->font;
    }

    std::string const & Element::getFontFace() const
    {
        auto element = this;
        while (element->fontFace.empty()) element = element->parent;
        return element->fontFace;
    }

    Measure const & Element::getFontSize() const
    {
        auto element = this;
        while (!element->fontSize.value && element->parent) element = element->parent;
        return element->fontSize;
    }

    FontStyle const & Element::getFontStyle() const
    {
        auto element = this;
        while (element->fontStyle == INHERIT) element = element->parent;
        return element->fontStyle;
    }

    int Element::getFontWeight() const
    {
        auto element = this;
        while (!element->fontWeight) element = element->parent;
        return element->fontWeight;
    }

    HWND Element::getInnerHwnd() const noexcept
    {
        return hwndInner;
    }

    HWND Element::getOuterHwnd() const noexcept
    {
        return hwndOuter;
    }

    Element * Element::getParent(bool const returnSelfIfRoot) noexcept
    {
        if (returnSelfIfRoot && !parent) return this;
        return parent;
    }

    Window * Element::getRoot() const noexcept
    {
        auto root = this;
        while (root->parent) root = root->parent;
        return (Window *)root;
    }

    Color Element::getTextColor() const noexcept
    {
        return textColor;
    }

    std::string const & Element::getValue() const
    {
        return value;
    }

    void Element::hide()
    {
        setVisible(false);
    }

    Measure Element::parseMeasure(std::string const & spec)
    {
        constexpr static auto const pattern = R"(^([0-9]+(?:\.?[0-9]+)?)(em|px|pt|%)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW(std::format("Invalid size: bad format. Expected N[.F][px|em|pt|%], saw {}", spec).c_str());

        double offset = atof(matches[1].str().c_str());
        std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
        auto unit = Unit::PX;
        if (unitStr == "em") unit = EM;
        else if (unitStr == "%") unit = PC;
        else if (unitStr == "pt") unit = PT;

        return { offset, unit };
    }

    size_t Element::recalculateLayout(bool * canMakeStuffUp)
    {
        size_t unresolved = 0;
        unresolved += recalculateDimension(INNER, WIDTH);
        unresolved += recalculateDimension(INNER, HEIGHT);
        unresolved += recalculateDimension(OUTER, WIDTH);
        unresolved += recalculateDimension(OUTER, HEIGHT);

        unresolved += recalculatePos(OUTER, TOP, canMakeStuffUp);
        unresolved += recalculatePos(OUTER, LEFT, canMakeStuffUp);
        unresolved += recalculatePos(OUTER, BOTTOM);
        unresolved += recalculatePos(OUTER, RIGHT);

        unresolved += recalculatePos(INNER, TOP, canMakeStuffUp);
        unresolved += recalculatePos(INNER, LEFT, canMakeStuffUp);
        unresolved += recalculatePos(INNER, BOTTOM);
        unresolved += recalculatePos(INNER, RIGHT);


        for (auto child : children)
        {
            unresolved += child.get()->recalculateLayout(canMakeStuffUp);
        }

        return unresolved;

        //TODO if (my update changes my size or position or that of my children) then (re-layout siblings)
    }

    Resolved Element::recalculatePos(Side const side, Edge const edge, bool * canMakeStuffUp)
    {
        if (futurePos[side].coord[edge].has_value()) return RESOLVED;

        if (futurePos[side].coord[~edge].has_value())
        {
            switch (edge)
            {
            case TOP:
                if (futurePos[side].size[HEIGHT].has_value())
                {
                    futurePos[side].coord[edge] = *(futurePos[side].coord[~edge]) - *(futurePos[side].size[HEIGHT]);
                    return RESOLVED;
                }
                break;

            case BOTTOM:
                if (futurePos[side].size[HEIGHT].has_value())
                {
                    futurePos[side].coord[edge] = *(futurePos[side].coord[~edge]) + *(futurePos[side].size[HEIGHT]);
                    return RESOLVED;
                }
                break;

            case LEFT:
                if (futurePos[side].size[WIDTH].has_value())
                {
                    futurePos[side].coord[edge] = *(futurePos[side].coord[~edge]) - *(futurePos[side].size[WIDTH]);
                    return RESOLVED;
                }
                break;

            case RIGHT:
                if (futurePos[side].size[WIDTH].has_value())
                {
                    futurePos[side].coord[edge] = *(futurePos[side].coord[~edge]) + *(futurePos[side].size[WIDTH]);
                    return RESOLVED;
                }
                break;
            }

            if (canMakeStuffUp && *canMakeStuffUp && (edge == TOP || edge == LEFT))
            {
                jaml_log(SEV_WARN, std::format("Element \"{}\" {} coordinate unresolved. Forcing to 0.", id, edgeToString(edge)).c_str());
                futurePos[side].coord[edge] = 0;
                return RESOLVED;
            }
        }

        auto tether = tethers[edge];
        // Tether to other element, or, if no tether specified, to sibling (top/left only)
        if (tether.edge != AUTO || edge == TOP || edge == LEFT)
        {
            Element * other = tether.id.empty()
                ? (i ? parent->getChild(i - 1) : nullptr)
                : getRoot()->findElement(tether.id);

            if (tether.edge == AUTO)
            {
                if (edge == LEFT) tether.edge = RIGHT; // adjacent to previous sibling
                else tether.edge = TOP; // same top as previous sibling
            }

            futurePos[side].coord[edge] = other ? other->futurePos[side].coord[tether.edge] : 0;

            if (futurePos[side].coord[edge].has_value())
            {
                return applyOffset(side, edge, tether.offset, canMakeStuffUp);
            }
        }
        if (canMakeStuffUp && *canMakeStuffUp && (edge == TOP || edge == LEFT))
        {
            jaml_log(SEV_WARN, std::format("Element \"{}\" {} coordinate unresolved. Forcing to 0.", id, edgeToString(edge)).c_str());
            futurePos[side].coord[edge] = 0;
            *canMakeStuffUp = false;
            return RESOLVED;
        }
        return UNRESOLVED;
    }

    Resolved Element::recalculateDimension(Side const side, Dimension const dim)
    {
        // Already calculated?
        if (futurePos[side].size[dim].has_value()) return RESOLVED;

        // From explicit coordinates
        auto const first = dim == WIDTH ? LEFT : TOP;
        auto const second = dim == WIDTH ? RIGHT : BOTTOM;
        if (futurePos[side].coord[first].has_value() && futurePos[side].coord[second].has_value())
        {
            futurePos[side].size[dim] = *(futurePos[side].coord[second]) - *(futurePos[side].coord[first]);
            return RESOLVED;
        }

        // From explicit size
        if (side == OUTER)
        {
            if (size[dim].unit != NONE)
            {
                futurePos[side].size[dim] = size[dim].toPixels(this->getParent(), dim);
                return futurePos[side].size[dim].has_value() ? RESOLVED : UNRESOLVED;
            }
        }
        if (futurePos[~side].size[dim].has_value())
        {
            auto const pad1 = padding[first].toPixels(this, dim, INNER);
            auto const pad2 = padding[second].toPixels(this, dim, INNER);
            if (pad1.has_value() && pad2.has_value())
            {
                futurePos[side].size[dim] = futurePos[~side].size[dim].value() - ((side==INNER?1:-1)*(pad1.value() + pad2.value()));
                return RESOLVED;
            }
        }

        // Auto ... of children
        if (side == OUTER) return UNRESOLVED;
        size_t max = 0;
        for (auto const child : children)
        {
            auto const cur = child.get()->futurePos[side].coord[second];
            if (!cur.has_value()) return UNRESOLVED;
            if (cur.value() > max) max = cur.value();
        }

        // ...of inherent content
        if (dim == HEIGHT)
        {
            // Height
            auto fontPx = getFontHeight(hwndInner);
            size_t mine = 0;
            if (!label.empty()) mine = fontPx;
            auto const hdc = GetDC(hwndInner);
            switch (type)
            {
            case STATIC:
                break;
            case EDIT:
                mine = fontPx;
                break;
            case CHECKBOX:
                mine = max(mine, 12 * GetDeviceCaps(hdc, LOGPIXELSY) / 96 + 1);
                break;
            case COMBOBOX:
                MX_THROW("COMBOBOX not yet implemented");
            case LISTBOX:
                MX_THROW("LISTBOX not yet implemented");
            }
            ReleaseDC(hwndInner, hdc);

            futurePos[side].size[dim] = max(mine, max);
        }
        else
        {
            // Width
            // TODO GetTextExtent various types
        }

        return RESOLVED;
    }

    void Element::removeChildren()
    {
        for (auto child : children)
        {
            child.get()->removeChildren();
            auto hwnd = child.get()->hwndInner;
            if (hwnd) DestroyWindow(hwnd);
            hwnd = child.get()->hwndOuter;
            if (hwnd) DestroyWindow(hwnd);
        }
        children.clear();
    }

    void Element::remove()
    {
        removeChildren();
        if (hwndInner) DestroyWindow(hwndInner);
        if (hwndOuter) DestroyWindow(hwndOuter);

        // Remove from parent
        parent->children.erase(parent->children.begin() + i, parent->children.begin() + i + 1);
        for (size_t x = 0; i < parent->children.size(); ++i)
        {
            parent->children.at(x).get()->i = x;
        }
    }

    void Element::setBackgroundColor(Color const & v)
    {
        if (backgroundColor.ref() == v.ref()) return;
        backgroundColor = v;
        updateBackgroundBrush();
    }
    
    void Element::setBackgroundColor(std::string const & spec)
    {
        setBackgroundColor(Color::parse(spec));
    }

    void Element::setFontFace(std::string_view const & face)
    {
        fontFace = face;
        updateFont();
    }

    void Element::setFontSize(Measure const & size)
    {
        fontSize = size;
        updateFont();
    }

    void Element::setFontSize(std::string const & spec)
    {
        fontSize = parseMeasure(spec);
        updateFont();
    }

    void Element::setFontStyle(FontStyle const & style)
    {
        fontStyle = style;
        updateFont();
    }

    void Element::setFontWeight(int const weight)
    {
        fontWeight = weight;
        updateFont();
    }

    void Element::setHeight(std::string const & spec)
    {
        size[HEIGHT] = parseMeasure(spec);
    }

    void Element::setId(std::string_view const & v)
    {
        id = v;
    }

    void Element::setImage(HBITMAP v)
    {
        //TODO
    }

    void Element::setLabel(std::string_view const & v)
    {
        label = v;
    }

    void Element::setOpacity(uint8_t const v)
    {
        if (opacity == v) return;
        opacity = v;
    }

    void Element::setPadding(Edge const edge, Measure const & v)
    {
        padding[edge] = v;
    }

    void Element::setTextAlignH(Edge const v)
    {
        textAlignH = v;
    }

    void Element::setTextColor(Color const & v)
    {
        if (textColor.ref() == v.ref()) return;
        textColor = v;
    }

    void Element::setTextColor(std::string const & spec)
    {
        textColor = Color::parse(spec);
    }

    void Element::setType(ElementType const v)
    {
        type = v;
    }

    void Element::setType(std::string_view const & v)
    {
        if (v == "button") { type = BUTTON; return; }
        if (v == "combobox") { type = COMBOBOX; return; }
        if (v == "edit") { type = EDIT; return; }
        if (v == "listbox") { type = LISTBOX; return; }
        if (v == "checkbox") { type = CHECKBOX; return; }
        type = STATIC;
    }

    void Element::setValue(std::string_view const & v)
    {
        value = v;
    }

    void Element::setVisible(bool const v)
    {
        visible = v;
    }

    void Element::setWidth(std::string const & spec)
    {
        size[WIDTH] = parseMeasure(spec);
    }

    void Element::show()
    {
        setVisible(true);
    }

    void Element::tether(Edge const mySide, std::string_view const & otherId, Edge const otherSide, Measure const & offset)
    {
        tethers[mySide] = { otherId, otherSide, offset };
        // TODO Validate that no tether has cyclic dependencies
    }

    void Element::tether(Edge const mySide, std::string const & spec)
    {
        constexpr static auto const pattern = R"(^([^>]+)>(left|right|bottom|top|l|r|t|b)(?:(\+|\-[0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?)?$)";
        constexpr static auto const simplePattern = R"(^(\-?[0-9]+(?:\.[0-9]+)?)(?:\s+)?(em|px|%)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);
        static auto const simpleRegex = std::regex(simplePattern, std::regex_constants::ECMAScript);
        
        auto matches = std::smatch{};
        if (std::regex_search(spec, matches, regex))
        {
            auto otherSide = mySide;
            switch (matches[2].str()[0])
            {
            case 't': otherSide = TOP; break;
            case 'l': otherSide = LEFT; break;
            case 'b': otherSide = BOTTOM; break;
            case 'r': otherSide = RIGHT; break;
            }
            if (isHEdge(mySide) != isHEdge(otherSide)) MX_THROW("Invalid tether: incompatible edge axis.");

            double offset = (matches.length() >= 3) ? atof(matches[3].str().c_str()) : 0;
            std::string const unitStr = (matches.length() >= 4) ? matches[4].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") { unit = PC; offset /= 100; }

            tethers[mySide] = { matches[1].str(), otherSide, { offset, unit } };
            // TODO Validate that no tether has cyclic dependencies
        }
        else if (parent && std::regex_search(spec, matches, simpleRegex))
        {
            double offset = atof(matches[1].str().c_str());
            if (mySide == BOTTOM || mySide == RIGHT) offset = -offset;
            std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
            auto unit = Unit::PX;
            if (unitStr == "em") unit = EM;
            else if (unitStr == "%") MX_THROW("Invalid tether: % not implemented.");
            tethers[mySide] = { parent->id, mySide, { offset, unit } };
        }
        else MX_THROW(std::format("Invalid tether: bad format. Expected [id>side][±offset em|px], saw {}", spec).c_str());
    }

    void Element::updateBackgroundBrush()
    {
        if (backgroundBrush) DeleteObject(backgroundBrush);
        backgroundBrush = CreateSolidBrush(getBackgroundColor().ref());
    }

    void Element::updateFont()
    {
        if (font)
        {
            DeleteObject(font);
            font = 0;
        }

        auto face = fontFace;
        auto size = fontSize;
        auto style = fontStyle;
        auto weight = fontWeight;
        if (face.empty() && !size.value && style == INHERIT && !weight)
        {
            return;
        }

        face = getFontFace();
        size = getFontSize();
        style = getFontStyle();
        weight = getFontWeight();

        static LOGFONT f;
        auto const hdc = GetDC(hwndInner);
        f.lfHeight = fontSize.toPixels(this, HEIGHT, INNER).value();
        ReleaseDC(hwndInner, hdc);
        f.lfWeight = weight;
        f.lfItalic = style & FontStyle::ITALIC;
        f.lfUnderline = style & FontStyle::UNDERLINE;
        f.lfStrikeOut = style & FontStyle::STRIKETHROUGH;
        f.lfCharSet = DEFAULT_CHARSET;
        memcpy(f.lfFaceName, Utf16String(face).data(), (LF_FACESIZE - 1) * sizeof(WCHAR));
        if (font) DeleteObject(font);
        font = CreateFontIndirect(&f);

        SendMessage(hwndInner, WM_SETFONT, (WPARAM)font, 0);
        SendMessage(hwndOuter, WM_SETFONT, (WPARAM)font, 0);
    }

    std::optional<int> Measure::toPixels(Element * context, Dimension const dim, Side const side) const
    {
        switch (unit)
        {
        case PX:
            return (int)value;

        case EM:
            return static_cast<int>(static_cast<double>(getFontHeight(context->getInnerHwnd())) * value);

        case PT:
        {
            auto hdc = GetDC(context->getInnerHwnd());
            return MulDiv((int)value, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        }

        case PC:
            if (context->futurePos[side].size[dim].has_value())
            {
                return ((double)(context->futurePos[side].size[dim].value())) * value;
            }
            return std::nullopt;
        }

        MX_THROW("Specified unit cannot be converted to pixels.");
    }

    Edge operator ~(Edge const edge)
    {
        switch (edge)
        {
        case TOP: return BOTTOM;
        case BOTTOM: return TOP;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        }
        MX_THROW("Specified edge has no opposite.");
    }

    Side operator ~(Side const side)
    {
        switch (side)
        {
        case INNER: return OUTER;
        case OUTER: return INNER;
        }
        MX_THROW("Invalid side specified.");
    }

    Dimension edgeToDimension(Edge const edge)
    {
        switch (edge)
        {
        case LEFT: case RIGHT: return WIDTH;
        case TOP: case BOTTOM: return HEIGHT;
        }
        MX_THROW("Invalid edge specified.");
    }

    int Window::start(HINSTANCE hInstance, int const nCmdShow)
    {
        g_hInstance = hInstance;

        WNDCLASSEXW wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = g_hInstance;
        wcex.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LEGOINVENTORYMANAGER2));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = CreateSolidBrush(RGB(0x9C, 0xD6, 0xE4));
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LEGOINVENTORYMANAGER2);
        wcex.lpszClassName = L"JAML_WINDOW";
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        RegisterClassExW(&wcex);

        hwndOuter = CreateWindowW(wcex.lpszClassName, Utf16String(label).c_str(), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, g_hInstance, nullptr);
        if (!hwndOuter) throw std::runtime_error("Failed to create outer parent window");

        RECT rect{};
        GetWindowRect(hwndOuter, &rect);
        currentPos[INNER].coord[TOP] = 0;
        currentPos[INNER].coord[LEFT] = 0;
        currentPos[INNER].coord[BOTTOM] = currentPos[INNER].size[HEIGHT] = rect.bottom - rect.top;
        currentPos[INNER].coord[RIGHT] = currentPos[INNER].size[WIDTH] = rect.right - rect.left;
        
        currentPos[OUTER] = currentPos[INNER];
        futurePos[INNER] = currentPos[INNER];
        futurePos[OUTER] = currentPos[OUTER];

        hwndInner = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, currentPos[OUTER].coord[LEFT].value(), currentPos[OUTER].coord[TOP].value(), currentPos[OUTER].size[WIDTH].value(), currentPos[OUTER].size[HEIGHT].value(), hwndOuter, NULL, g_hInstance, NULL);
        if (!hwndOuter) throw std::runtime_error("Failed to create inner parent window");

        updateFont();

        size_t unresolved = (std::numeric_limits<size_t>::max)();
        while (unresolved)
        {
            auto const previous = unresolved;
            unresolved = recalculateLayout();
            if (unresolved == previous)
            {
                if (throwOnUnresolved)
                {
                    MX_THROW("Failed to resolve one or more coordinates => cyclic tether dependency!");
                }
                bool canMakeStuffUp = true;
                unresolved = recalculateLayout(&canMakeStuffUp);
                if (unresolved == previous) MX_THROW("Failed to resolve any coordinates despite force-resolve!")
            }
        }

        commitLayout();

        ShowWindow(hwndOuter, nCmdShow);
        UpdateWindow(hwndOuter);

        HACCEL hAccelTable = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDC_LEGOINVENTORYMANAGER2));

        MSG msg;


        // Main message loop:
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return (int)msg.wParam;
    }

    void JamlParser::eat_whitespace()
    {
        while(pos < source.size())
        {
            switch (source[pos])
            {
            case '\n':
                ++pos;
                ++line;
                col = 1;
                continue;
            case ' ':
            case '\t':
            case '\r':
                ++pos;
                ++col;
                continue;
            }
            break;
        }
    }

    std::string_view JamlParser::parse_key()
    {
        auto const start = pos;
        while(pos < source.size())
        {
            if (source[pos] != '-' && (source[pos] < 'a' || source[pos] > 'z'))
            {
                return { source.begin() + start, source.begin() + pos };
            }
            ++pos;
            ++col;
        }
        MX_THROW(std::format("JAML parse error: Unexpected end of input at {},{}", line, col).c_str());
    }

    std::string JamlParser::parse_val()
    {
        auto oss = std::ostringstream{};
        bool const quoted = source[pos] == '"';
        if (quoted) { ++pos; ++col; }
        for (; pos < source.size(); ++pos, ++col)
        {
            switch (source[pos])
            {
            case '\r':
            case '\n':
            case ' ':
            case '\t':
            case '}':
            case '=':
            case ';':
                if (!quoted)
                {
                    ++pos; ++col;
                    return oss.str();
                }
                if (source[pos] == '\r' || source[pos] == '\n') MX_THROW(std::format("JSON parse error: Unexpected end-of-line at {},{}", line, col).c_str());
                oss << source[pos];
                break;
            case '"':
                if (quoted)
                {
                    ++pos; ++col;
                    return oss.str();
                }
                MX_THROW(std::format("JAML parse error: Unexpected '\"' in property value at {},{}", line, col).c_str());

            case ('\\'):
                ++pos; ++col;
                if (pos >= source.size()) MX_THROW(std::format("JSON parse error: Unexpected end of input at {},{}", line, col).c_str());
                switch (source[pos])
                {
                case '"':  oss << '"'; continue;
                case '\\': oss << '\\'; continue;
                case 'b': oss << '\b'; continue;
                case 'f': oss << '\f'; continue;
                case 'n': oss << '\n'; continue;
                case 'r': oss << '\r'; continue;
                case 't': oss << '\t'; continue;
                case 'u':
                    // TODO
                    [[fallthrough]];
                default:
                    MX_THROW(std::format("JAML parse error: Unsupported escape sequence \\{} at {},{}.", source[pos], line, col).c_str());
                }
            default:
                oss << source[pos];
                break;
            }
        }
        MX_THROW(std::format("JAML parse error: Unexpected end of input at {},{}.", line, col).c_str());
    }

    void JamlParser::parse_node()
    {
        if (pos >= source.size() || source[pos] != '{') MX_THROW("JAML parse error: Node must start with '{'");
        ++pos; ++col;

        while (pos < source.size())
        {
            eat_whitespace();
            if (pos >= source.size()) break;

            switch (source[pos])
            {
            case '{':
                {
                    elem = elem->addChild();
                    parse_node();
                    continue;
                }

            case '}':
                ++pos; ++col;
                return;

            default:
                auto key = parse_key();
                eat_whitespace();
                if (source[pos] != '=') MX_THROW(std::format("JAML parse error: Missing '=' at {},{}", line, col).c_str());
                ++pos; ++col;
                eat_whitespace();
                auto val = parse_val();
                if (key == "id") elem->setId(val);
                else if (key == "value") elem->setValue(val);
                else if (key == "label") elem->setLabel(val);
                else if (key == "type") elem->setType(val);
                else if (key == "left") elem->tether(LEFT, val);
                else if (key == "right") elem->tether(RIGHT, val);
                else if (key == "top") elem->tether(TOP, val);
                else if (key == "bottom") elem->tether(BOTTOM, val);
                else if (key == "width") elem->setWidth(val);
                else if (key == "height") elem->setHeight(val);
                else if (key == "fontface") elem->setFontFace(val);
                else if (key == "fontsize") elem->setFontSize(val);
                else if (key == "color") elem->setTextColor(val);
                else if (key == "background-color") elem->setBackgroundColor(val);
                else MX_THROW(std::format("JAML parse error: Unknown property name \"{}\"", key));
                continue;
            }
        }

        MX_THROW(std::format("JAML parse error: Unexpected end of input at {},{}.", line, col).c_str());
    }

    Element::Element(std::string_view const & id)
    {
        this->id = id;
    }

    JamlParser::JamlParser(std::string_view const & source, Window * window) : source(source), elem(window)
    {
        if (source.empty() || source[0] != '{') MX_THROW("JAML input must start with '{'");
        parse_node();
    };

    void Window::defaultFont()
    {
        fontFace = "Arial";
        fontWeight = REGULAR;
        fontStyle = NORMAL;
        fontSize = { 12, PT };
    }

    Window::Window()
    {
        defaultFont();
    }

    Window::Window(std::string_view const & jamlSource)
    {
        defaultFont();
        JamlParser parser(jamlSource, this);
    }

    Window::Window(std::filesystem::path const & file)
    {
        FILE * f = fopen(file.string().c_str(), "r");

        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        auto jamlSource = std::string{};
        jamlSource.resize(size);

        rewind(f);
        fread(jamlSource.data(), sizeof(char), size, f);

        defaultFont();
        JamlParser parser({ jamlSource }, this);
    }

    void Window::setForceResolve(bool const force)
    {
        throwOnUnresolved = !force;
    }

}