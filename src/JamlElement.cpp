#include "MxiLogging.h"
#include "MxiUtils.h"

#include "JamlElement.h"

namespace jaml
{

    namespace
    {
        static void draw_nc_frame(HDC  hdc, RECT * rect, BOOL  active, DWORD style, DWORD ex_style)
        {
            INT width, height;

            if (style & WS_THICKFRAME)
            {
                width = get_system_metrics(SM_CXFRAME) - get_system_metrics(SM_CXDLGFRAME);
                height = get_system_metrics(SM_CYFRAME) - get_system_metrics(SM_CYDLGFRAME);

                NtGdiSelectBrush(hdc, get_sys_color_brush(active ? COLOR_ACTIVEBORDER :
                    COLOR_INACTIVEBORDER));
                /* Draw frame */
                NtGdiPatBlt(hdc, rect->left, rect->top, rect->right - rect->left, height, PATCOPY);
                NtGdiPatBlt(hdc, rect->left, rect->top, width, rect->bottom - rect->top, PATCOPY);
                NtGdiPatBlt(hdc, rect->left, rect->bottom - 1, rect->right - rect->left, -height, PATCOPY);
                NtGdiPatBlt(hdc, rect->right - 1, rect->top, -width, rect->bottom - rect->top, PATCOPY);

                InflateRect(rect, -width, -height);
            }

            /* Now the other bit of the frame */
            if ((style & (WS_BORDER | WS_DLGFRAME)) || (ex_style & WS_EX_DLGMODALFRAME))
            {
                DWORD color;

                width = get_system_metrics(SM_CXDLGFRAME) - get_system_metrics(SM_CXEDGE);
                height = get_system_metrics(SM_CYDLGFRAME) - get_system_metrics(SM_CYEDGE);
                /* This should give a value of 1 that should also work for a border */

                if (ex_style & (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE)) color = COLOR_3DFACE;
                else if (ex_style & WS_EX_STATICEDGE) color = COLOR_WINDOWFRAME;
                else if (style & (WS_DLGFRAME | WS_THICKFRAME)) color = COLOR_3DFACE;
                else color = COLOR_WINDOWFRAME;
                NtGdiSelectBrush(hdc, get_sys_color_brush(color));

                /* Draw frame */
                NtGdiPatBlt(hdc, rect->left, rect->top,
                    rect->right - rect->left, height, PATCOPY);
                NtGdiPatBlt(hdc, rect->left, rect->top,
                    width, rect->bottom - rect->top, PATCOPY);
                NtGdiPatBlt(hdc, rect->left, rect->bottom - 1,
                    rect->right - rect->left, -height, PATCOPY);
                NtGdiPatBlt(hdc, rect->right - 1, rect->top,
                    -width, rect->bottom - rect->top, PATCOPY);

                InflateRect(rect, -width, -height);
            }
        }

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
                BITMAP bm;
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

    void JamlElement::registerClass(HINSTANCE hInstance)
    {
        WNDCLASSEXW wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = JamlElement_WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
        wcex.lpszClassName = L"JAML_ELEMENT";
        RegisterClassExW(&wcex);
    }

    LRESULT JamlElement::paint(HWND hwnd, HDC hdc)
    {
            //SetTextColor(hdc, elem->getTextColor().ref());
            //SetBkColor(hdc, elem->getBackgroundColor().ref());
            auto hbrush = getBackgroundBrush();
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

            if (hFont = getFont())
                hOldFont = SelectObject(hdc, hFont);

            if ((style & SS_TYPEMASK) != SS_SIMPLE)
            {
                FillRect(hdc, &rc, hbrush);
                if (!IsWindowEnabled(hwnd)) SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
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

    LRESULT JamlElement_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

        auto elem = (JamlElement *)GetPropA((HWND)lParam, "elem");
        if (!elem)
        {
            MX_LOG_ERROR("JamlElement_WndProc called for non-JamlElement.");
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
            return (LRESULT)elem->getFont();

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

    void JamlElement::Build(ClassMap & classes)
    {
        for (auto & cp : classes)
        {
            auto c = cp.second.get();
            if (c->GetElement()) continue;
            if (c->GetName().starts_with('.')) continue;
            if (c->GetParentName() != m_name) continue;
            auto ep = std::make_shared<JamlElement>(JamlElement{ c->GetName() });
            m_children.push_back(ep);
            auto e = ep.get();
            c->SetElement(e);
            e->m_parent = this;
            e->classes.push_back(std::string{ c->GetName() });
            e->classes.insert(e->classes.end(), c->GetClassNames().begin(), c->GetClassNames().end());
            e->Build(classes);
        }
    }

    JamlWindow const * JamlElement::GetWindow() const
    {
        auto window = this;
        while (window->m_parent) { window = window->m_parent; }
        return (JamlWindow*)window;
    }

    std::optional<Tether> const & JamlElement::GetTether(Edge const edge) const
    {
        auto const window = GetWindow();
        for (auto const cname : classes)
        {
            auto const c = window->GetClass(cname);
            auto const tether = c->GetTether(edge);
            if (!tether.has_value()) continue;
            return tether;
        }
    }

    void JamlElement::PrepareToComputeLayout()
    {
        m_futureRect = {};
        for (auto child : m_children)
        {
            child.get()->PrepareToComputeLayout();
        }
    }

    size_t JamlElement::ComputeLayout(bool * canMakeStuffUp)
    {
        size_t unresolved = 0;
        unresolved += ComputeEdge(TOP, canMakeStuffUp);
        unresolved += ComputeEdge(LEFT, canMakeStuffUp);
        unresolved += ComputeEdge(BOTTOM, canMakeStuffUp);
        unresolved += ComputeEdge(RIGHT, canMakeStuffUp);
        unresolved += ComputeSize(WIDTH, canMakeStuffUp);
        unresolved += ComputeSize(HEIGHT, canMakeStuffUp);
        for (auto child : m_children)
        {
            child.get()->ComputeLayout(canMakeStuffUp);
        }
        return unresolved;
    }

    Resolved JamlElement::ComputeEdge(Edge const edge, bool * canMakeStuffUp)
    {
        if (m_futureRect.HasEdge(edge)) return RESOLVED;

        auto const tether = GetTether(edge);
        if (tether.has_value())
        {
            auto const offset = tether.value().offset.toPixels(this, edgeToDimension(edge));
            if (!offset.has_value()) return UNRESOLVED;
            if (tether.value().id.empty())
            {
                // Tether to parent
                int anchor = 0;
                if (isFarEdge(tether.value().edge))
                {
                    if (m_parent->ComputeEdge(edge, canMakeStuffUp) == RESOLVED)
                    {
                        anchor = m_parent->m_futureRect.GetEdge(tether.value().edge);
                    }
                    else
                    {
                        return UNRESOLVED;
                    }
                }
                m_futureRect.SetEdge(edge, anchor + offset.value());
                return RESOLVED;
            }

            // Tether to sibling
            // TODO
        }

        // Auto, use margin from sibling
        // TODO

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
}