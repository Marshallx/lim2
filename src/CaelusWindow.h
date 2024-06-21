#pragma once

#include "CaelusClass.h"
#include "CaelusElement.h"

namespace Caelus
{
    constexpr static auto const kWindowClass = L"Caelus_WINDOW";
    LRESULT CALLBACK CaelusWindow_WndProc(HWND, UINT, WPARAM, LPARAM);

    class CaelusWindow : public CaelusElement
    {
    public:
        CaelusWindow();
        CaelusWindow(std::filesystem::path const & file);
        CaelusWindow(std::string_view const & source);
        CaelusClassMap const & GetClassMap() const;
        static void Register(HINSTANCE hInstance);
        int Start(HINSTANCE hInstance, int const nCmdShow, int const x = CW_USEDEFAULT, int const y = CW_USEDEFAULT, int width = CW_USEDEFAULT, int height = CW_USEDEFAULT);
        void IgnoreErrors(bool const ignore = true);
        void SetResizable(bool const resizable = true);

    private:
        CaelusWindow(CaelusWindow const &) = delete;
        void SetDefaults();
        void BuildAll();
        void FitToOuter();
        void FitToInner();
        bool m_throwOnUnresolved = true;
        CaelusClassMap m_definedClasses = {};
        bool m_resizable = false;
        HWND m_outerHwnd = 0;

    };

}