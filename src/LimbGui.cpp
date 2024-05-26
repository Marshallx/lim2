#include <stdexcept>

#include <Windows.h>
#include <CommCtrl.h>

#include "..\resource\Resource.h"

#include "jaml.h"
#include "Limb.h"

namespace mx
{
    /*
    void LimbGui::test()
    {
        HANDLE hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ADDPART), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

        HWND hStatic = CreateWindowW(
            L"STATIC",
            L"Add Part",
            WS_TABSTOP | SS_BITMAP | WS_VISIBLE | WS_CHILD,
            10, 10,
            80, 80,
            hwnd_,
            NULL,
            hInst_,
            NULL
        );

        SendMessage(
            (HWND)hStatic,
            (UINT)STM_SETIMAGE,
            (WPARAM)IMAGE_BITMAP,
            (LPARAM)hBmp
        );

        HWND hButton = CreateWindowW(
            L"BUTTON",
            L"Add Part",
            WS_TABSTOP | BS_BITMAP | WS_VISIBLE | WS_CHILD,
            90, 90,
            80, 80,
            hwnd_,
            NULL,
            hInst_,
            NULL
        );

        SendMessage(
            (HWND)hButton,
            (UINT)BM_SETIMAGE,
            (WPARAM)IMAGE_BITMAP,
            (LPARAM)hBmp
        );

    }

    void LimbGui::Init(HINSTANCE hInstance, int nCmdShow)
    {
        hInst_ = hInstance;
        nCmdShow_ = nCmdShow;

        LoadStringW(hInstance, IDS_APP_TITLE, title_, MAX_LOADSTRING);
        LoadStringW(hInstance, IDC_LEGOINVENTORYMANAGER2, windowClass_, MAX_LOADSTRING);

        WNDCLASSEXW wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEGOINVENTORYMANAGER2));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = CreateSolidBrush(RGB(0x9C, 0xD6, 0xE4));
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LEGOINVENTORYMANAGER2);
        wcex.lpszClassName = windowClass_;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        RegisterClassExW(&wcex);

        hwnd_ = CreateWindowW(windowClass_, title_, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

        if (!hwnd_) throw std::runtime_error("Failed to create window");

        CreateQueryBar();
        test();
    }
    */
 

}