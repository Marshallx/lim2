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

    

    Element * Element::addChild(std::string_view const & id)
    {
        auto const child = std::make_shared<Element>(Element{});
        child.get()->id = id.empty() ? create_guid() : id;
        child.get()->parent = this;
        child.get()->i = m_children.size();
        m_children.push_back(child);
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


        for (auto child : m_children)
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
        for (auto const & child : m_children)
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
        return m_children.at(i).get();
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


        for (auto child : m_children)
        {
            unresolved += child.get()->recalculateLayout(canMakeStuffUp);
        }

        return unresolved;

        //TODO if (my update changes my size or position or that of my children) then (re-layout siblings)
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
        for (auto const child : m_children)
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
        for (auto child : m_children)
        {
            child.get()->removeChildren();
            auto hwnd = child.get()->hwndInner;
            if (hwnd) DestroyWindow(hwnd);
            hwnd = child.get()->hwndOuter;
            if (hwnd) DestroyWindow(hwnd);
        }
        m_children.clear();
    }

    void Element::remove()
    {
        removeChildren();
        if (hwndInner) DestroyWindow(hwndInner);
        if (hwndOuter) DestroyWindow(hwndOuter);

        // Remove from parent
        parent->m_children.erase(parent->m_children.begin() + i, parent->m_children.begin() + i + 1);
        for (size_t x = 0; i < parent->m_children.size(); ++i)
        {
            parent->m_children.at(x).get()->i = x;
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

    

    Element::Element(std::string_view const & id)
    {
        this->id = id;
    }

    

   

}