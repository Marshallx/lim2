#include <filesystem>
#include <fstream>
#include <iostream>

#include "CaelusElement.h"
#include "CaelusParser.h"

#include "CaelusWindow.h"

namespace Caelus
{
    void CaelusWindow::SetDefaults()
    {
        auto cp = std::make_shared<CaelusClass>("window");
        auto c = cp.get();
        m_definedClasses.m_map["window"] = cp;
        c->SetBackgroundColor(0xFFFFFF);
        c->SetFontFace("Arial");
        c->SetFontSize("12pt");
        c->SetFontStyle("normal");
        c->SetFontWeight(REGULAR);
        c->SetTextColor(0);
    }

    CaelusWindow::CaelusWindow()
    {
        SetDefaults();
    }

    CaelusWindow::CaelusWindow(std::string_view const & CaelusSource)
    {
        SetDefaults();
        CaelusParser parser(CaelusSource, m_definedClasses);
    }

    CaelusWindow::CaelusWindow(std::filesystem::path const & file)
    {
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
    }

    CaelusClassMap const & CaelusWindow::GetClassMap() const
    {
        return m_definedClasses;
    }

    void CaelusWindow::IgnoreErrors(bool const ignore)
    {
        m_throwOnUnresolved = !ignore;
    }

    int CaelusWindow::Start(HINSTANCE hInstance, int const nCmdShow)
    {
        m_client = new CaelusElement("window");
        m_client->Build();

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
                    MX_THROW("Element \"{}\" parent \"{}\" is also a descendant.");
                }
            }
            MX_THROW("Element \"{}\" parent \"{}\" not found.");
        }

        m_client->PrepareToComputeLayout();
        size_t previousUnresolvedCount = 0;
        for(;;)
        {
            size_t currentUnresolvedCount = m_client->ComputeLayout();
            if (currentUnresolvedCount == 0) break;
            if (previousUnresolvedCount == currentUnresolvedCount)
            {
                MX_THROW("Failed to recalculate layout - cyclic dependency?");
            }
        }

        m_client->CommitLayout(hInstance);

        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);


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