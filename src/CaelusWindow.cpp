#include <filesystem>
#include <fstream>
#include <iostream>

#include "CaelusElement.h"
#include "jaml.h"
#include "jass.h"

#include "CaelusWindow.h"

namespace Caelus
{
    void CaelusWindow::Register(HINSTANCE hInstance)
    {
        WNDCLASS wndclass = {
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = CaelusWindow_WndProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = LoadIcon(NULL, IDI_APPLICATION),
            .hCursor = LoadCursor(NULL, IDC_ARROW),
            .hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
            .lpszMenuName = NULL,
            .lpszClassName = kWindowClass,
        };
        RegisterClass(&wndclass);
    }

    LRESULT CaelusWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //HDC hdc;
        //PAINTSTRUCT ps;
        //RECT rect;
        switch (msg)
        {
        case WM_CREATE:
        case WM_PAINT:
            /*
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            DrawTextW(hdc, (L"Hello,Windows"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&rect, 2);
            EndPaint(hwnd, &ps);
            */

            return DefWindowProc(hwnd, msg, wparam, lparam);

        case WM_NCCREATE:
            SetPropA(hwnd, "CaelusWindow", this);
            m_outerHwnd = hwnd;
            break;

        case WM_SIZING:
        {
            auto r = (RECT *)lparam;
            Relayout(r->right - r->left, r->bottom - r->top);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    LRESULT CALLBACK CaelusWindow_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (!IsWindow(hwnd)) return 0;

        auto that = (CaelusWindow *)((msg != WM_NCCREATE)
            ? GetPropA(hwnd, "CaelusWindow")
            : ((CREATESTRUCTW *)lparam)->lpCreateParams
        );

        return that->WndProc(hwnd, msg, wparam, lparam);
    }

    CaelusWindow::CaelusWindow() : CaelusWindow(std::string_view{ "<jaml><head></head><body></body></jaml>" }) {}
    CaelusWindow::CaelusWindow(std::string_view const & source) : CaelusElement("window")
    {
        Init();
        JamlParser(source, *this);
        BuildAll();
    }

    CaelusWindow::CaelusWindow(std::filesystem::path const & file) : CaelusElement("window")
    {
        auto const source = mxi::file_get_contents(file);

        Init();
        JamlParser(std::string_view{ source }, *this);
        BuildAll();
    }

    void CaelusWindow::IgnoreErrors(bool const ignore)
    {
        m_throwOnUnresolved = !ignore;
    }

    void CaelusWindow::BuildAll()
    {

        for (auto & child : m_children)
        {
            child.Build();
            if (child.m_tagname == "head")
            {
                for (auto tag : child.m_children)
                {
                    if (tag.m_tagname == "style")
                    {
                        JassParser{ tag.m_text, m_rules };
                    }
                    else if (tag.m_tagname == "link")
                    {
                        auto const href = std::filesystem::path(tag.m_attributes["href"]);
                        if (href.empty() || !std::filesystem::exists(href))
                        {
                            MX_LOG_WARN(std::format("Link file not found: {}", href));
                        }
                        else
                        {
                            if (tag.m_attributes["rel"] == "stylesheet" || href.extension() == ".css")
                            {
                                auto const css = mxi::file_get_contents(href);
                                JassParser{ css, m_rules };
                            }
                        }
                    }
                }
            }
            else if (child.m_tagname != "body")
            {
                MX_THROW("Unexpected tag \"{}\". Expected \"head\" or \"body\".", child.m_tagname);
            }
        }
    }

    void CaelusWindow::FitToInner(HWND inner)
    {
        auto outerHwnd = ::GetParent(inner);
        auto rcClient = RECT{};
        auto rcWind = RECT{};
        auto ptDiff = POINT{};
        GetClientRect(inner, &rcClient);
        auto width = rcClient.right;
        auto height = rcClient.bottom;
        GetClientRect(outerHwnd, &rcClient);
        GetWindowRect(outerHwnd, &rcWind);
        ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
        ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
        width += ptDiff.x;
        height += ptDiff.y;

        if (rcWind.right - rcWind.left != width ||
            rcWind.bottom - rcWind.top != height)
        {
            SetWindowPos(outerHwnd, NULL, rcWind.left, rcWind.top,
                width, height,
                SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }

    void CaelusWindow::FitToOuter()
    {
        // TODO - On outer resize, relayout
        /*
        auto rcClient = RECT{};
        auto rcWind = RECT{};
        auto ptDiff = POINT{};
        GetClientRect(m_outerHwnd, &rcClient);
        GetWindowRect(m_outerHwnd, &rcWind);
        ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
        ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

        SetWindowPos(m_hwnd, NULL, 0, 0,
            rcWind.right - ptDiff.x,
            rcWind.bottom - ptDiff.y,
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

        */
    }

    int CaelusWindow::Start(HINSTANCE hInstance, int const nCmdShow, int const x, int const y, int const width, int const height)
    {
        auto const & optTitle = GetLabel();
        auto const title = optTitle.has_value() ? optTitle.value() : std::string{};

        auto hwnd = CreateWindow(
            kWindowClass,
            mxi::Utf16String(title).c_str(),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            x == -1 ? 100 : x,
            y == -1 ? 100 : y,
            width == -1 ? 640 : width,
            height == -1 ? 480 : height,
            NULL,
            NULL,
            hInstance,
            this
        );

        if (!hwnd || hwnd != m_outerHwnd) MX_THROW("Failed to create element window!");

        Relayout(width, height);

        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);


        // Main message loop:
        //HACCEL hAccelTable = CreateAcceleratorTable(hInstance, MAKEINTRESOURCE(IDC_LEGOINVENTORYMANAGER2));
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return (int)msg.wParam;
    }

    void CaelusWindow::Relayout(int const width, int const height)
    {
        PrepareToComputeLayout();

        m_futureRect.SetEdge(Edge::TOP, 0);
        m_futureRect.SetEdge(Edge::LEFT, 0);
        auto r = RECT{};
        GetClientRect(m_outerHwnd, &r);
        if (width != -1) m_futureRect.SetEdge(Edge::RIGHT, r.right);
        if (height != -1) m_futureRect.SetEdge(Edge::BOTTOM, r.bottom);

        size_t previousUnresolvedCount = 0;
        for (;;)
        {
            size_t currentUnresolvedCount = ComputeLayout();
            if (currentUnresolvedCount == 0) break;
            if (previousUnresolvedCount == currentUnresolvedCount)
            {
                MX_THROW("Failed to recalculate layout - cyclic dependency?");
            }
            previousUnresolvedCount = currentUnresolvedCount;
        }

        //auto hdwp = BeginDeferWindowPos(10);
        /*hdwp = */CommitLayout(GetModuleHandle(NULL), /*hdwp*/NULL, m_outerHwnd);
        //if (!hdwp || !EndDeferWindowPos(hdwp))
        //{
            //MX_THROW(std::format("EndDeferWindowPos failed: {}", GetLastError()));
        //}
    }
}