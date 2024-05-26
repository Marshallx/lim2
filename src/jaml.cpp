#include <deque>
#include <fstream>
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

    int getFontHeight(HWND hwnd, HFONT font)
    {
        TEXTMETRIC tm = {};
        auto const hdc = GetDC(hwnd);
        auto const r = GetTextMetrics(hdc, &tm);
        ReleaseDC(hwnd, hdc);
        return tm.tmHeight;
    }

    int getLineHeight(HWND hwnd, HFONT font)
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

    bool isHSide(Side const side)
    {
        return side == LEFT || side == RIGHT;
    }

    bool isVSide(Side const side)
    {
        return side == TOP || side == BOTTOM;
    }

    char peek(std::string_view const & str, size_t pos)
    {
        if (pos < str.size()) return str[pos];
        return 0;
    }

    std::string_view eat_whitespace(std::string_view const & source)
    {
        size_t pos = 0;
        for (; pos < source.size(); ++pos)
        {
            switch (source[pos])
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\0':
                continue;
            }
            return { source.begin() + pos, source.end() };
        }
        return {};
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
            static HBRUSH hBrush = CreateSolidBrush(RGB(230, 0, 230));
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 0, 0));
            SetBkColor(hdcStatic, RGB(230, 0, 230));
            return (INT_PTR)hBrush;
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
        child.get()->id = id;
        child.get()->parent = this;
        child.get()->i = children.size();
        children.push_back(child);
        return child.get();
    }

    size_t Element::applyOffset(Side const side, Measure const & offset)
    {
        switch (offset.unit)
        {
        case PX:
            futurePos.coord[side] = futurePos.coord[side].value() + static_cast<int>(offset.value);
            return 0;

        case EM:
            futurePos.coord[side] = futurePos.coord[side].value() + static_cast<int>(
                static_cast<double>(getFontHeight(hwnd, font)) * offset.value);
            return 0;

        case PC:
            auto const of = (side == TOP || side == BOTTOM) ? parent->futurePos.height : parent->futurePos.width;
            if (of.has_value())
            {
                futurePos.coord[side] = futurePos.coord[side].value() + static_cast<int>(
                    static_cast<double>(of.value()) * offset.value);
                return 0;
            }
            break;
        }

        futurePos.coord[side].reset();
        return 1;
    }

    void Element::commitLayout()
    {
        currentPos = futurePos;
        futurePos = {};

        if (!hwnd)
        {
            create();
        }
        else
        {
            // Reposition everything
            SetWindowPos(hwnd, 0,
                currentPos.coord[LEFT].value(),
                currentPos.coord[TOP].value(),
                currentPos.width.value(),
                currentPos.height.value(),
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
        if (hwnd) throw std::runtime_error("Element window already created!");
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

        hwnd = CreateWindow(winClassName, Utf16String(label).c_str(), style, 10, 10, 200, 12, parent->hwnd, NULL, g_hInstance, NULL);
        if (!hwnd) throw std::runtime_error("Failed to create element window");
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

    HWND Element::getHwnd() const noexcept
    {
        return hwnd;
    }

    Window * Element::getRoot() const noexcept
    {
        auto root = this;
        while (root->parent) root = root->parent;
        return (Window *)root;
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
        constexpr static auto const pattern = R"(^([0-9]+(?:\.?[0-9]+)?)(em|px|%)?)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW("Invalid tether: bad format.");

        double offset = atof(matches[1].str().c_str());
        std::string const unitStr = (matches.length() >= 2) ? matches[2].str() : "";
        auto unit = Unit::PX;
        if (unitStr == "em") unit = EM;
        else if (unitStr == "%") unit = PC;
        else if (unitStr == "pt") unit = PT;

        return { offset, unit };
    }

    size_t Element::recalculateHeight()
    {
        if (futurePos.height.has_value()) return 0;

        if (futurePos.coord[TOP].has_value() && futurePos.coord[BOTTOM].has_value())
        {
            futurePos.width = *(futurePos.coord[BOTTOM]) - *(futurePos.coord[TOP]);
            return 0;
        }

        // TODO get height from explicit
        /*
        switch (size.width.unit)
        {
        case AUTO:
        case PX:
        case EM:
        case PC:
        }
        */
        // TODO get height from children/content
        return 1;
    }

    size_t Element::recalculateLayout()
    {
        //re-layout

        size_t unresolved = 0;
        unresolved += recalculatePos(Side::TOP);
        unresolved += recalculatePos(Side::LEFT);
        unresolved += recalculatePos(Side::BOTTOM);
        unresolved += recalculatePos(Side::RIGHT);

        for (auto child : children)
        {
            unresolved += child.get()->recalculateLayout();
        }

        return unresolved;

        //TODO if (my update changes my size or position or that of my children) then (re-layout siblings)
    }

    size_t Element::recalculatePos(Side const side)
    {
        if (futurePos.coord[side].has_value()) return 0;

        if (futurePos.coord[~side].has_value())
        {
            switch (side)
            {
            case TOP:
                if (futurePos.height.has_value())
                {
                    futurePos.coord[side] = *(futurePos.coord[~side]) - *(futurePos.height);
                    return 0;
                }
                return 1;

            case BOTTOM:
                if (futurePos.height.has_value())
                {
                    futurePos.coord[side] = *(futurePos.coord[~side]) + *(futurePos.height);
                    return 0;
                }
                return 1;

            case LEFT:
                if (futurePos.width.has_value())
                {
                    futurePos.coord[side] = *(futurePos.coord[~side]) - *(futurePos.width);
                    return 0;
                }
                return 1;

            case RIGHT:
                if (futurePos.width.has_value())
                {
                    futurePos.coord[side] = *(futurePos.coord[~side]) + *(futurePos.width);
                    return 0;
                }
                return 1;
            }
        }

        auto tether = tethers[side];
        Element * other = tether.id.empty()
            ? (i ? parent->getChild(i - 1) : nullptr)
            : getRoot()->findElement(tether.id);

        futurePos.coord[side] = other ? other->futurePos.coord[TOP] : 0;

        if (futurePos.coord[side].has_value())
        {
            return applyOffset(side, tether.offset);
        }
        return 1;
    }

    size_t Element::recalculateWidth()
    {
        if (futurePos.width.has_value()) return 0;

        if (futurePos.coord[LEFT].has_value() && futurePos.coord[RIGHT].has_value())
        {
            futurePos.width = *(futurePos.coord[RIGHT]) - *(futurePos.coord[LEFT]);
            return 0;
        }

        // TODO get width from explicit
        // TODO get width from children/content
        return 1;
    }

    void Element::removeChildren()
    {
        for (auto child : children)
        {
            child.get()->removeChildren();
            auto const hwnd = child.get()->hwnd;
            if (hwnd) DestroyWindow(hwnd);
        }
        children.clear();
    }

    void Element::remove()
    {
        removeChildren();
        if (hwnd) DestroyWindow(hwnd);

        // Remove from parent
        parent->children.erase(parent->children.begin() + i, parent->children.begin() + i + 1);
        for (size_t x = 0; i < parent->children.size(); ++i)
        {
            parent->children.at(x).get()->i = x;
        }
    }

    void Element::setBackgroundColor(Color const & v)
    {
        if (backgroundColor.value() == v.value()) return;
        backgroundColor = v;
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
        size.height = parseMeasure(spec);
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

    void Element::setPadding(Side const side, Measure const & v)
    {
        // TODO
        padding[side] = v;
    }

    void Element::setTextAlignH(Side const v)
    {
        textAlignH = v;
    }

    void Element::setTextColor(Color const & v)
    {
        if (textColor.value() == v.value()) return;
        textColor = v;
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
        size.width = parseMeasure(spec);
    }

    void Element::show()
    {
        setVisible(true);
    }

    void Element::tether(Side const mySide, std::string_view const & otherId, Side const otherSide, Measure const & offset)
    {
        tethers[mySide] = { otherId, otherSide, offset };
        // TODO Validate that no tether has cyclic dependencies
    }

    void Element::tether(Side const mySide, std::string const & spec)
    {
        // Example: id>left+5px

        constexpr static auto const pattern = R"(^([^>]+)>(left|right|bottom|top|l|r|t|b)(?:(\+|\-[0-9]+(?:\.?[0-9]+)?)(em|px|%)?)?$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript);
        
        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW("Invalid tether: bad format.");
        
        auto otherSide = mySide;
        switch (matches[2].str()[0])
        {
        case 't': otherSide = TOP; break;
        case 'l': otherSide = LEFT; break;
        case 'b': otherSide = BOTTOM; break;
        case 'r': otherSide = RIGHT; break;
        }
        if (isHSide(mySide) != isHSide(otherSide)) MX_THROW("Invalid tether: incompatible side axis.");

        double offset = (matches.length() >= 3) ? atof(matches[3].str().c_str()) : 0;
        std::string const unitStr = (matches.length() >= 4) ? matches[4].str() : "";
        auto unit = Unit::PX;
        if (unitStr == "em") unit = EM;
        else if (unitStr == "%") unit = PC;
        
        tethers[mySide] = { matches[1].str(), otherSide, { offset, unit } };
        // TODO Validate that no tether has cyclic dependencies
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
        auto const hdc = GetDC(hwnd);
        f.lfHeight = fontSize.toPixels(this);
        ReleaseDC(hwnd, hdc);
        f.lfWeight = weight;
        f.lfItalic = style & FontStyle::ITALIC;
        f.lfUnderline = style & FontStyle::UNDERLINE;
        f.lfStrikeOut = style & FontStyle::STRIKETHROUGH;
        f.lfCharSet = DEFAULT_CHARSET;
        memcpy(f.lfFaceName, Utf16String(face).data(), (LF_FACESIZE - 1) * sizeof(WCHAR));
        if (font) DeleteObject(font);
        font = CreateFontIndirect(&f);
    }

    int Measure::toPixels(Element const * element) const
    {
        switch (unit)
        {
        case PX:
            return (int)value;

        case EM:
        case PC:
            return static_cast<int>(static_cast<double>(getFontHeight(element->getHwnd(), element->getFont())) * value);

        case PT:
            auto hdc = GetDC(element->getHwnd());
            return -MulDiv((int)value, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        }

        MX_THROW("Specified unit cannot be converted to pixels.");
    }

    Side operator ~(Side const side)
    {
        switch (side)
        {
        case TOP: return BOTTOM;
        case BOTTOM: return TOP;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        }
        MX_THROW("Specified side has no opposite.");
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

        hwnd = CreateWindowW(wcex.lpszClassName, Utf16String(label).c_str(), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, g_hInstance, nullptr);

        if (!hwnd) throw std::runtime_error("Failed to create parent window");

        RECT rect{};
        GetWindowRect(hwnd, &rect);
        currentPos.coord[LEFT] = rect.left;
        currentPos.coord[TOP] = rect.top;
        currentPos.coord[RIGHT] = rect.right;
        currentPos.coord[BOTTOM] = rect.bottom;
        currentPos.width = rect.right - rect.left;
        currentPos.height = rect.bottom - rect.top;
        futurePos = currentPos;

        size_t unresolved = 1;
        while (unresolved)
        {
            auto const previous = unresolved;
            unresolved = recalculateLayout();
            if (unresolved == previous)
            {
                throw std::runtime_error("Failed to resolve one or more coordinates => cyclic tether dependency!");
            }
        }

        commitLayout();

        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);

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

    std::tuple<std::string_view, std::string, std::string_view> jaml_parser_read_property(std::string_view & source)
    {
        source = eat_whitespace(source);
        size_t pos = 0;
        for (; pos < source.size(); ++pos)
        {
            if (source[pos] < 'a' || source[pos] > 'z') break;
        }
        if (pos + 2 >= source.size()) MX_THROW("Unexpected end of input.");
        if (source[pos] != '=') MX_THROW("Missing = after property name.");
        size_t valPos = pos + 1;
        if (source[valPos] != '"') MX_THROW("Missing property value (expected opening \")");

        size_t cchParsed = 0;
        auto const propval = json_unescape_string({ source.begin() + valPos, source.end() }, &cchParsed);

        return {
            { source.begin(), source.begin() + pos },
            propval,
            { source.begin() + valPos + cchParsed, source.end() }
        };
    }

    std::string_view jaml_parser_parse_node(std::string_view const & source, Element * element)
    {
        if (peek(source) != '{') MX_THROW("JAML input must start with {.");
        std::string_view working = { source.begin() + 1, source.end() };
        while (!working.empty())
        {
            working = eat_whitespace(working);
            if (working.empty()) break;

            switch (working[0])
            {
            case '{':
                {
                    auto child = element->addChild();
                    working = jaml_parser_parse_node(working, child);
                    continue;
                }
            case '}':
                return { working.begin() + 1, working.end() };
            default:
                auto r = jaml_parser_read_property(working);
                auto propname = std::get<0>(r);
                auto propval = std::get<1>(r);
                working = std::get<2>(r);
                if (propname == "id") element->setId(propval);
                else if (propname == "value") element->setValue(propval);
                else if (propname == "label") element->setLabel(propval);
                else if (propname == "type") element->setType(propval);
                else if (propname == "left") element->tether(LEFT, propval);
                else if (propname == "right") element->tether(RIGHT, propval);
                else if (propname == "top") element->tether(TOP, propval);
                else if (propname == "bottom") element->tether(BOTTOM, propval);
                else if (propname == "width") element->setWidth(propval);
                else if (propname == "height") element->setHeight(propval);
                else if (propname == "fontface") element->setFontFace(propval);
                else if (propname == "fontsize") element->setFontSize(propval);
                else MX_THROW(std::format("Unknown JAML property name {}", propname));
                continue;
            }
        }

        throw std::runtime_error("Unexpected end of JAML input.");
    }

    Element::Element(std::string_view const & id)
    {
        this->id = id;
    }

    Window::Window(std::string const & source) : Window()
    {
        jaml_parser_parse_node(source, this);
    }

    Window::Window(std::filesystem::path const & file) : Window()
    {
        FILE * f = fopen(file.string().c_str(), "r");

        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        auto source = std::string{};
        source.resize(size);

        rewind(f);
        fread(source.data(), sizeof(char), size, f);

        jaml_parser_parse_node(source, this);
    }

}