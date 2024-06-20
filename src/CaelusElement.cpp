#include "MxiLogging.h"
#include "MxiUtils.h"

#include "CaelusElement.h"

namespace Caelus
{

    namespace
    {

        HRGN set_control_clipping(HDC hdc, const RECT * rect)
        {
            RECT rc = *rect;
            HRGN hrgn = CreateRectRgn(0, 0, 0, 0);

            if (GetClipRgn(hdc, hrgn) != 1)
            {
                DeleteObject(hrgn);
                hrgn = 0;
            }
            DPtoLP(hdc, (POINT *)&rc, 2);
            if (GetLayout(hdc) & LAYOUT_RTL)  // compensate for the shifting done by IntersectClipRect
            {
                rc.left++;
                rc.right++;
            }
            IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
            return hrgn;
        }

        static BOOL hasTextStyle(DWORD style)
        {
            switch (style & SS_TYPEMASK)
            {
            case SS_SIMPLE:
            case SS_LEFT:
            case SS_LEFTNOWORDWRAP:
            case SS_CENTER:
            case SS_RIGHT:
            case SS_OWNERDRAW:
                return TRUE;
            }

            return FALSE;
        }

        // TODO move to paint func
        static void STATIC_PaintBitmapfn(HWND hwnd, HDC hdc, HBRUSH hbrush, DWORD style)
        {
            HDC hMemDC;
            HBITMAP hBitmap, oldbitmap;

            if ((hBitmap = (HBITMAP)GetWindowLongPtrW(hwnd, HICON_GWL_OFFSET))
                && (GetObjectType(hBitmap) == OBJ_BITMAP)
                && (hMemDC = CreateCompatibleDC(hdc)))
            {
                BITMAP bm = {};
                RECT rcClient;

                GetObjectW(hBitmap, sizeof(bm), &bm);
                oldbitmap = SelectObject(hMemDC, hBitmap);

                GetClientRect(hwnd, &rcClient);
                if (style & SS_CENTERIMAGE)
                {
                    hbrush = CreateSolidBrush(GetPixel(hMemDC, 0, 0));

                    FillRect(hdc, &rcClient, hbrush);

                    rcClient.left = (rcClient.right - rcClient.left) / 2 - bm.bmWidth / 2;
                    rcClient.top = (rcClient.bottom - rcClient.top) / 2 - bm.bmHeight / 2;
                    rcClient.right = rcClient.left + bm.bmWidth;
                    rcClient.bottom = rcClient.top + bm.bmHeight;

                    DeleteObject(hbrush);
                }
                StretchBlt(hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
                    rcClient.bottom - rcClient.top, hMemDC,
                    0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                SelectObject(hMemDC, oldbitmap);
                DeleteDC(hMemDC);
            }
        }
    }

    void CaelusElement::Register(HINSTANCE hInstance)
    {
        WNDCLASSEXW wcex = {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = CaelusElement_WndProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF)),
            .lpszClassName = L"Caelus_ELEMENT",
        };
        RegisterClassExW(&wcex);
    }

    void CaelusElement::DrawBorder(HDC hdc)
    {
        auto hbrush = CreateSolidBrush(GetBorderColor().rgb());
        NtGdiPatBlt(hdc, rect->left, rect->top,
                rect->right - rect->left, height, PATCOPY);
            NtGdiPatBlt(hdc, rect->left, rect->top,
                width, rect->bottom - rect->top, PATCOPY);
            NtGdiPatBlt(hdc, rect->left, rect->bottom - 1,
                rect->right - rect->left, -height, PATCOPY);
            NtGdiPatBlt(hdc, rect->right - 1, rect->top,
                -width, rect->bottom - rect->top, PATCOPY);
    }

    LRESULT CaelusElement::paint(HWND hwnd, HDC hdc)
    {
            //SetTextColor(hdc, elem->getTextColor().ref());
            //SetBkColor(hdc, elem->getBackgroundColor().ref());
            auto hbrush = m_drawing.GetBackgroundBrush();
            auto style = GetWindowLongW(hwnd, GWL_STYLE);

            RECT rc;
            HFONT hFont = NULL;
            HGDIOBJ hOldFont = NULL;
            UINT format;
            INT len, buf_size;
            WCHAR * text;

            GetClientRect(hwnd, &rc);

            switch (style & SS_TYPEMASK)
            {
            case SS_LEFT:
                format = DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK;
                break;

            case SS_CENTER:
                format = DT_CENTER | DT_EXPANDTABS | DT_WORDBREAK;
                break;

            case SS_RIGHT:
                format = DT_RIGHT | DT_EXPANDTABS | DT_WORDBREAK;
                break;

            case SS_SIMPLE:
                format = DT_LEFT | DT_SINGLELINE;
                break;

            case SS_LEFTNOWORDWRAP:
                format = DT_LEFT | DT_EXPANDTABS;
                break;

            default:
                return;
            }

            if (GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_RIGHT)
                format = DT_RIGHT | (format & ~(DT_LEFT | DT_CENTER));

            if (style & SS_NOPREFIX)
                format |= DT_NOPREFIX;

            if ((style & SS_TYPEMASK) != SS_SIMPLE)
            {
                if (style & SS_CENTERIMAGE)
                    format |= DT_SINGLELINE | DT_VCENTER;
                if (style & SS_EDITCONTROL)
                    format |= DT_EDITCONTROL;
                if (style & SS_ENDELLIPSIS)
                    format |= DT_SINGLELINE | DT_END_ELLIPSIS;
                if (style & SS_PATHELLIPSIS)
                    format |= DT_SINGLELINE | DT_PATH_ELLIPSIS;
                if (style & SS_WORDELLIPSIS)
                    format |= DT_SINGLELINE | DT_WORD_ELLIPSIS;
            }

            if (hFont = GetHfont()) hOldFont = SelectObject(hdc, hFont);

            if ((style & SS_TYPEMASK) != SS_SIMPLE)
            {
                FillRect(hdc, &rc, hbrush);
            }

            buf_size = 256;
            if (!(text = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, buf_size * sizeof(WCHAR))))
                goto no_TextOut;

            while ((len = InternalGetWindowText(hwnd, text, buf_size)) == buf_size - 1)
            {
                buf_size *= 2;
                if (!(text = (WCHAR*)HeapReAlloc(GetProcessHeap(), 0, text, buf_size * sizeof(WCHAR))))
                    goto no_TextOut;
            }

            if (!len) goto no_TextOut;

            if (((style & SS_TYPEMASK) == SS_SIMPLE) && (style & SS_NOPREFIX))
            {
                ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,
                    &rc, text, len, NULL);
            }
            else
            {
                DrawTextW(hdc, text, -1, &rc, format);
            }

        no_TextOut:
            HeapFree(GetProcessHeap(), 0, text);

            if (hFont)
                SelectObject(hdc, hOldFont);
    }

    LRESULT CaelusElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lResult = 0;
        LONG full_style = GetWindowLongW(hwnd, GWL_STYLE);
        LONG style = full_style & SS_TYPEMASK;

        if (!IsWindow(hwnd)) return 0;

        if (uMsg == WM_CREATE)
        {
            if (style < 0L || style > SS_TYPEMASK)
            {
                MX_LOG_ERROR("Unknown static style.");
                return -1;
            }
            return lResult;
        }

        auto elem = (CaelusElement *)GetPropA((HWND)lParam, "elem");
        if (!elem)
        {
            MX_LOG_ERROR("CaelusElement_WndProc called for non-CaelusElement.");
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        switch (uMsg)
        {

        case WM_NCDESTROY:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        case WM_ERASEBKGND:
            return 1; // Do all painting in WM_PAINT, like Windows does

        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT rect;
            GetClientRect(hwnd, &rect);
            HDC hdc = wParam ? HDC(wParam) : BeginPaint(hwnd, &ps);
            HRGN hrgn = set_control_clipping(hdc, &rect);
            elem->paint(hwnd, hdc); break;
            SelectClipRgn(hdc, hrgn);
            if (hrgn) DeleteObject(hrgn);
            if (!wParam) EndPaint(hwnd, &ps);
        }
        break;

        case WM_NCCREATE:
        {
            // Only for top level windows (ala dialogs)
            CREATESTRUCTW * cs = (CREATESTRUCTW *)lParam;

            if (full_style & SS_SUNKEN || style == SS_ETCHEDHORZ || style == SS_ETCHEDVERT)
            {
                SetWindowLongW(hwnd, GWL_EXSTYLE, GetWindowLongW(hwnd, GWL_EXSTYLE) | WS_EX_STATICEDGE);
            }

            if (style == SS_ETCHEDHORZ || style == SS_ETCHEDVERT)
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                if (style == SS_ETCHEDHORZ) rc.bottom = rc.top; else rc.right = rc.left;
                AdjustWindowRectEx(&rc, full_style, FALSE, GetWindowLongW(hwnd, GWL_EXSTYLE));
                SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
            }

            switch (style) {
            case SS_ICON:
            {
                const WCHAR * name = cs->lpszName;
                HICON hIcon;

                if (name && name[0] == 0xffff)
                {
                    name = MAKEINTRESOURCEW(name[1]);
                }

                hIcon = STATIC_LoadIconW(cs->hInstance, name, full_style);
                STATIC_SetIcon(hwnd, hIcon, full_style);
            }
            break;
            case SS_BITMAP:
                if ((ULONG_PTR)cs->hInstance >> 16)
                {
                    const WCHAR * name = cs->lpszName;
                    HBITMAP hBitmap;

                    if (name && name[0] == 0xffff)
                    {
                        name = MAKEINTRESOURCEW(name[1]);
                    }

                    hBitmap = LoadBitmapW(cs->hInstance, name);
                    STATIC_SetBitmap(hwnd, hBitmap, full_style);
                }
                break;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

        case WM_SETTEXT:
            if (hasTextStyle(full_style))
            {
                lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
                STATIC_TryPaintFcn(hwnd, full_style);
            }
            break;

        case WM_SETFONT:
            return 0; // Ignore

        case WM_GETFONT:
            return (LRESULT)elem->m_drawing.GetHfont();

        case WM_NCHITTEST:
            if (full_style & SS_NOTIFY)
                return HTCLIENT;
            else
                return HTTRANSPARENT;

        case WM_GETDLGCODE:
            return DLGC_STATIC;

        case WM_LBUTTONDOWN:
        case WM_NCLBUTTONDOWN:
            if (full_style & SS_NOTIFY)
                SendMessageW(GetParent(hwnd), WM_COMMAND,
                    MAKEWPARAM(GetWindowLongPtrW(hwnd, GWLP_ID), STN_CLICKED), (LPARAM)hwnd);
            return 0;

        case WM_LBUTTONDBLCLK:
        case WM_NCLBUTTONDBLCLK:
            if (full_style & SS_NOTIFY)
                SendMessageW(GetParent(hwnd), WM_COMMAND,
                    MAKEWPARAM(GetWindowLongPtrW(hwnd, GWLP_ID), STN_DBLCLK), (LPARAM)hwnd);
            return 0;

        case STM_GETIMAGE:
            return (LRESULT)STATIC_GetImage(hwnd, wParam, full_style);

        case STM_GETICON:
            return (LRESULT)STATIC_GetImage(hwnd, IMAGE_ICON, full_style);

        case STM_SETIMAGE:
            switch (wParam) {
            case IMAGE_BITMAP:
                if (style != SS_BITMAP) return 0;
                lResult = (LRESULT)STATIC_SetBitmap(hwnd, (HBITMAP)lParam, full_style);
                break;
            case IMAGE_ICON:
            case IMAGE_CURSOR:
                if (style != SS_ICON) return 0;
                lResult = (LRESULT)STATIC_SetIcon(hwnd, (HICON)lParam, full_style);
                break;
            }
            STATIC_TryPaintFcn(hwnd, full_style);
            break;

        case STM_SETICON:
            if (style != SS_ICON) return 0;
            lResult = (LRESULT)STATIC_SetIcon(hwnd, (HICON)wParam, full_style);
            STATIC_TryPaintFcn(hwnd, full_style);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        return lResult;
    }

    // =-=-=-=-=-=-=-=-= Layout =-=-=-=-=-=-=-=-=

    void CaelusElement::Build()
    {
        auto window = GetWindow();
        for (auto & cp : window->GetClassMap().m_map)
        {
            auto c = cp.second.get();
            if (c->GetElement()) continue;
            if (c->GetName().starts_with('.')) continue;
            if (c->GetParentName() != m_name) continue;
            auto e = AppendChild(c->GetName());
            c->SetElement(e);
            e->m_parent = this;
            e->Build();
        }
    }

    Resolved CaelusElement::ComputeBorder(Edge const edge)
    {
        if (m_futureRect.HasBorder(edge)) return RESOLVED;
        auto const borderDef = GetBorderWidthDef(edge);
        if (!borderDef) return RESOLVED;
        auto borderOpt = borderDef->toPixels(this, edgeToDimension(edge));
        if (!borderOpt.has_value()) return UNRESOLVED;
        m_futureRect.SetBorder(edge, borderOpt.value());
        return RESOLVED;
    }

    Resolved CaelusElement::ComputeEdge(Edge const edge)
    {
        if (m_futureRect.HasEdge(edge)) return RESOLVED;

        auto tether = GetTether(edge);
        auto const defaultTether = GetDefaultTether(edge);
        if (!tether) tether = &defaultTether;
        auto const offset = tether->offset.toPixels(this, edgeToDimension(edge));
        if (!offset.has_value()) return UNRESOLVED;
        int anchor = 0;
        int nc = 0;
        if (tether->id.empty() || tether->id == m_parent->m_name)
        {
            // Tether to parent (interior)
            if (m_parent->ComputeNC(tether->edge) == UNRESOLVED) return UNRESOLVED;
            nc = m_parent->m_futureRect.GetNC(tether->edge);
            if (isFarEdge(tether->edge))
            {
                if (m_parent->ComputeSize(edgeToDimension(tether->edge)) == UNRESOLVED) return UNRESOLVED;
                anchor = m_parent->m_futureRect.GetSize(edgeToDimension(tether->edge)) - nc;
            }
            else
            {
                anchor = nc;
            }
        }
        else
        {
            // Tether to sibling (adjacent or named) (exterior)
            auto sibling = (tether->id == ".") ? GetSibling(edge) : GetSibling(tether->id);
            auto otherEdge = (tether->id == "." && sibling == m_parent) ? edge : tether->edge;
            if (sibling != m_parent || isFarEdge(otherEdge))
            {
                if (sibling->ComputeEdge(otherEdge) == UNRESOLVED) return UNRESOLVED;
                anchor = sibling->m_futureRect.GetEdge(otherEdge);
            }
        }

        m_futureRect.SetEdge(edge, anchor + offset.value());
        return RESOLVED;
    }

    size_t CaelusElement::ComputeLayout()
    {
        size_t unresolved = 0;
        unresolved += ComputePadding(TOP);
        unresolved += ComputePadding(LEFT);
        unresolved += ComputePadding(BOTTOM);
        unresolved += ComputePadding(RIGHT);
        unresolved += ComputeEdge(TOP);
        unresolved += ComputeEdge(LEFT);
        unresolved += ComputeEdge(BOTTOM);
        unresolved += ComputeEdge(RIGHT);
        unresolved += ComputeSize(WIDTH);
        unresolved += ComputeSize(HEIGHT);
        for (auto & child : m_children)
        {
            unresolved += child.get()->ComputeLayout();
        }
        return unresolved;
    }

    Resolved CaelusElement::ComputeNC(Edge const edge)
    {
        auto const r1 = ComputeBorder(edge);
        auto const r2 = ComputePadding(edge);
        return (r1 == RESOLVED && r2 == RESOLVED) ? RESOLVED : UNRESOLVED;
    }

    Resolved CaelusElement::ComputePadding(Edge const edge)
    {
        if (m_futureRect.HasPadding(edge)) return RESOLVED;
        auto const paddingDef = GetPaddingDef(edge);
        if (!paddingDef) return RESOLVED;
        auto paddingOpt = paddingDef->toPixels(this, edgeToDimension(edge));
        if (!paddingOpt.has_value()) return UNRESOLVED;
        m_futureRect.SetPadding(edge, paddingOpt.value());
        return RESOLVED;
    }

    Resolved CaelusElement::ComputeSize(Dimension const dim)
    {
        if (m_futureRect.HasSize(dim)) return RESOLVED;

        // NB: Don't call ComputeEdge() here -> infinite loop!

        auto const nearEdge = (dim == HEIGHT) ? TOP : LEFT;
        auto const farEdge = (dim == HEIGHT) ? BOTTOM : RIGHT;
        auto const nearTether = GetTether(nearEdge);
        auto const farTether = GetTether(farEdge);

        if (nearTether && farTether) return UNRESOLVED; // We are tethered on both sides. Size will be resolved when tethers are resolved.

        auto const sizeDef = GetSizeDef(dim);
        if (sizeDef)
        {
            // Explicit size
            auto sizeOpt = sizeDef->toPixels(this, dim);
            if (!sizeOpt.has_value()) return UNRESOLVED;
            m_futureRect.SetSize(dim, sizeOpt.value());
            return RESOLVED;
        }

        // TODO: min-size, max-size, entangled size

        // Auto size (from content)
        int furthestCoord = 0;
        for (auto & cp : m_children)
        {
            auto child = cp.get();
            if (!child->m_futureRect.HasEdge(farEdge)) return UNRESOLVED;
            auto farCoord = child->m_futureRect.GetEdge(farEdge);
            // Far margin, if any
            auto childTether = child->GetTether(farEdge);
            if (childTether && childTether->id == ".") farCoord -= childTether->offset.toPixels(child, dim).value();
            if (farCoord > furthestCoord) furthestCoord = farCoord;
        }
        m_futureRect.SetInnerSize(dim, furthestCoord);
        return RESOLVED;
    }

    void CaelusElement::PrepareToComputeLayout()
    {
        m_futureRect = {};
        for (auto & child : m_children)
        {
            child.get()->PrepareToComputeLayout();
        }
    }

    void CaelusElement::CommitLayout(HINSTANCE hInstance)
    {
        m_currentRect = m_futureRect;

        if (!m_hwnd)
        {
            Spawn(hInstance);
        }
        else
        {
            SetWindowPos(
                m_hwnd,
                NULL,
                m_currentRect.GetEdge(LEFT),
                m_currentRect.GetEdge(TOP),
                m_currentRect.GetSize(WIDTH),
                m_currentRect.GetSize(HEIGHT),
                SWP_NOZORDER
            );
        }

        for (auto & child : m_children)
        {
            child.get()->CommitLayout(hInstance);
        }
    }

    void CaelusElement::Spawn(HINSTANCE hInstance)
    {
        if (m_hwnd) MX_THROW("Element window already created!");
        auto * winClassName = L"STATIC";
        DWORD style = WS_CHILD | WS_VISIBLE;
        switch (GetTextAlignH())
        {
        case LEFT: style |= ES_LEFT; break;
        case ALL: style |= ES_CENTER; break;
        case RIGHT: style |= ES_RIGHT; break;
        }

        m_hwnd = CreateWindow(
            L"Caelus_ELEMENT",
            mxi::Utf16String(GetLabel()).c_str(),
            style,
            m_currentRect.GetEdge(LEFT),
            m_currentRect.GetEdge(TOP),
            m_currentRect.GetSize(WIDTH),
            m_currentRect.GetSize(HEIGHT),
            m_parent ? m_parent->m_hwnd : NULL,
            NULL,
            hInstance,
            NULL
        );

        if (!m_hwnd) MX_THROW("Failed to create element window!");

        SetPropA(m_hwnd, "elem", this);

        UpdateFont();
    }

    HFONT CaelusElement::GetHfont()
    {
        if (!m_hfont) UpdateFont();
        return m_hfont;
    }

    void CaelusElement::UpdateFont()
    {
        auto face = GetFontFace();
        auto size = GetFontSize();
        auto italic = GetFontItalic();
        auto weight = GetFontWeight();

        static LOGFONT f;
        auto const hdc = GetDC(m_hwnd);
        f.lfHeight = size.toPixels(this, HEIGHT).value();
        ReleaseDC(m_hwnd, hdc);
        f.lfWeight = weight;
        f.lfItalic = italic;
        f.lfUnderline = 0;
        f.lfStrikeOut = 0;
        f.lfCharSet = DEFAULT_CHARSET;
        memcpy(f.lfFaceName, mxi::Utf16String(face).data(), (LF_FACESIZE - 1) * sizeof(WCHAR));
        if (m_hfont) DeleteObject(m_hfont);
        m_hfont = CreateFontIndirect(&f);

        //SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hfont, 0); // Ignored
    }

    std::optional<int> CaelusElement::MeasureToPixels(Measure const * measure, Dimension const dim) const
    {
        switch (measure->unit)
        {
        case PX:
            return (int)measure->value;

        case EM:
            return static_cast<int>(static_cast<double>(getFontHeight(m_hwnd)) * measure->value);

        case PT:
        {
            auto hdc = GetDC(m_hwnd);
            return MulDiv((int)measure->value, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        }

        case PC:
            if (m_futureRect.GetSize(dim))
            {
                return ((double)(m_futureRect.GetSize(dim))) * measure->value;
            }
            return std::nullopt;
        }

        MX_THROW("Unsupported unit for conversion to pixels.");
    }

    // =-=-=-=-=-=-=-=-= Getters =-=-=-=-=-=-=-=-=

#define CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(FUNC) \
    auto const & CaelusElement::FUNC() const \
    { \
        auto elem = this; \
        auto map = GetWindow()->GetClassMap(); \
        while (elem) \
        { \
            auto const v = map.FUNC(this->m_name); \
            if (v.has_value()) return v.value(); \
            elem = this->m_parent; \
        } \
        MX_THROW(#FUNC " failed for element!"); \
    }

    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetBackgroundColor);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetFontFace);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetFontItalic);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetFontSize);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetFontWeight);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetLabel);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetTextAlignH);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION(GetTextColor);

#define CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION_WITH_EDGE(FUNC) \
    auto const & CaelusElement::FUNC(Edge const edge) const \
    { \
        auto elem = this; \
        auto map = GetWindow()->GetClassMap(); \
        while (elem) \
        { \
            auto const v = map.FUNC(edge, this->m_name); \
            if (v.has_value()) return v; \
            elem = this->m_parent; \
        } \
        MX_THROW(#FUNC " failed for element!"); \
    }

    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION_WITH_EDGE(GetBorderWidthDef);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION_WITH_EDGE(GetPaddingDef);
    CAELUS_DEFINE_GET_INHERITABLE_ELEMENT_STYLE_FUNCTION_WITH_EDGE(GetTether);

    CaelusElement * CaelusElement::FindElement(std::string_view const & name)
    {
        if (m_name == name) return this;
        for (auto const & child : m_children)
        {
            auto const found = child.get()->FindElement(name);
            if (found) return found;
        }
        return nullptr;
    }

    CaelusElement * CaelusElement::GetChild(size_t const n) noexcept
    {
        return (m_children.size() > n) ? m_children[n].get() : nullptr;
    }

    Tether const CaelusElement::GetDefaultTether(Edge const edge)
    {
        return { ".", ~edge, {0, PX} };
    }

    HWND CaelusElement::GetHwnd() const noexcept
    {
        return m_hwnd;
    }

    CaelusElement * CaelusElement::GetParent() noexcept
    {
        return m_parent;
    }

    CaelusElement * CaelusElement::GetSibling(std::string_view const & name) const
    {
        for (auto const & sibling : m_parent->m_children)
        {
            if (sibling.get()->m_name == name) return sibling.get();
        }
        return nullptr;
    }

    CaelusElement * CaelusElement::GetSibling(Edge const edge) const
    {
        for (size_t i = 0; i < m_parent->m_children.size(); ++i)
        {
            auto & sibling = m_parent->m_children[i];
            if (sibling.get() == this)
            {
                if (i == 0 || i == m_parent->m_children.size() - 1)
                {
                    return m_parent;
                }
                if (isFarEdge(edge)) return m_parent->m_children[++i].get();
                return m_parent->m_children[--i].get();
            }
        }
        return m_parent;
    }

    CaelusWindow const * CaelusElement::GetWindow() const
    {
        auto window = this;
        while (window->m_parent) { window = window->m_parent; }
        return (CaelusWindow*)window;
    }

    // =-=-=-=-=-=-=-=-= Setters =-=-=-=-=-=-=-=-=

    CaelusElement * CaelusElement::AppendChild(std::string_view const & name)
    {
        if (name.find_first_of('. ') != std::string::npos) MX_THROW("Element names cannot contain '.' or ' '.");
        auto ep = std::make_shared<CaelusElement>(CaelusElement{ name });
        m_children.push_back(ep);
        return ep.get();
    }

    CaelusElement * CaelusElement::InsertChild(std::string_view const & name, size_t n)
    {
        auto ep = std::make_shared<CaelusElement>(CaelusElement{ name });
        m_children.insert(std::next(m_children.begin(), n), ep);
        return ep.get();
    }

    void CaelusElement::Remove()
    {
        RemoveChildren();
        if (!m_parent) MX_THROW("Element::Remove called on Window");
        for (size_t n = 0; n < m_parent->m_children.size(); ++n)
        {
            if (m_parent->m_children[n].get() == this)
            {
                m_parent->RemoveChild(n);
                return;
            }
        }
    }

    void CaelusElement::RemoveChild(size_t const n)
    {
        m_children.erase(std::next(m_children.begin(), n));
    }

    void CaelusElement::RemoveChildren()
    {
        m_children.clear();
    }
}