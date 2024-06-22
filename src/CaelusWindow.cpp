#include <filesystem>
#include <fstream>
#include <iostream>

#include "CaelusElement.h"
#include "CaelusParser.h"

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

    LRESULT CALLBACK CaelusWindow_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        //HDC hdc;
        //PAINTSTRUCT ps;
        //RECT rect;
        switch (message)
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

            return DefWindowProc(hwnd, message, wParam, lParam);
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    void CaelusWindow::SetDefaults()
    {
        auto cp = std::make_shared<CaelusClass>("window", &m_definedClasses);
        auto c = cp.get();
        m_definedClasses.m_map["window"] = cp;
        m_class = c;
        c->SetElement(this);
        // Styles
        c->SetBackgroundColor(0xFFFFFF);
        c->SetBorderColor(0);
        c->SetBorderRadius("0");
        c->SetBorderWidth("0");
        c->SetFontFace("Arial");
        c->SetFontSize("12pt");
        c->SetFontStyle("normal");
        c->SetFontWeight(REGULAR);
        c->SetPadding("0");
        c->SetTextAlignH(LEFT);
        c->SetTextAlignV(TOP);
        c->SetTextColor(0);
    }

    CaelusWindow::CaelusWindow() : CaelusElement("window")
    {
        SetDefaults();
    }

    CaelusWindow::CaelusWindow(std::string_view const & CaelusSource) : CaelusElement("window")
    {
        SetDefaults();
        CaelusParser parser(CaelusSource, m_definedClasses);
        BuildAll();
    }

    CaelusWindow::CaelusWindow(std::filesystem::path const & file) : CaelusElement("window")
    {
        if (!std::filesystem::exists(file)) MX_THROW("Specified ANUS file does not exist.");

        FILE * f = fopen(file.string().c_str(), "r");

        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        auto CaelusSource = std::string{};
        CaelusSource.resize(size);

        rewind(f);
        fread(CaelusSource.data(), sizeof(char), size, f);

        SetDefaults();
        CaelusParser parser({ CaelusSource }, m_definedClasses);
        BuildAll();
    }

    CaelusClassMap const & CaelusWindow::GetClassMap() const
    {
        return m_definedClasses;
    }

    void CaelusWindow::IgnoreErrors(bool const ignore)
    {
        m_throwOnUnresolved = !ignore;
    }

    void CaelusWindow::BuildAll()
    {
        Build();

        // Check that all elements were built
        for (auto & cp : m_definedClasses.m_map)
        {
            auto c = cp.second.get();
            if (c->GetElement()) continue;
            if (cp.first.starts_with('.')) continue;
            for (auto & cp2 : m_definedClasses.m_map)
            {
                if (cp2.first == c->GetParentName())
                {
                    throw std::runtime_error(std::format("Element \"{}\" parent \"{}\" is also a descendant.", cp.first, cp2.first));
                }
            }
            throw std::runtime_error(std::format("Element \"{}\" parent \"{}\" not found.", cp.first, c->GetParentName()));
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
        HWND hwnd = CreateWindow(
            kWindowClass,
            mxi::Utf16String(title).c_str(),
            WS_OVERLAPPEDWINDOW,
            x,
            y,
            width,
            height,
            NULL,
            NULL,
            hInstance,
            NULL
        );

        PrepareToComputeLayout();

        m_futureRect.SetEdge(TOP, 0);
        m_futureRect.SetEdge(LEFT, 0);
        m_futureRect.SetEdge(RIGHT, width);
        m_futureRect.SetEdge(BOTTOM, height);

        size_t previousUnresolvedCount = 0;
        for(;;)
        {
            size_t currentUnresolvedCount = ComputeLayout();
            if (currentUnresolvedCount == 0) break;
            if (previousUnresolvedCount == currentUnresolvedCount)
            {
                MX_THROW("Failed to recalculate layout - cyclic dependency?");
            }
        }

        CommitLayout(hInstance, hwnd);

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

    /*
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
        wcex.lpszClassName = L"Caelus_WINDOW";
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

        
    }
    */
}