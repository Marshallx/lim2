#pragma once

#include "jaml.h"
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
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        CaelusClassMap const & GetClassMap() const;
        static void Register(HINSTANCE hInstance);
        int Start(HINSTANCE hInstance, int const nCmdShow, int const x = 100, int const y = 100, int width = 640, int height = 480);
        void Relayout(int const width, int const height);
        void IgnoreErrors(bool const ignore = true);
        void SetResizable(bool const resizable = true);
        static void FitToInner(HWND inner);

    private:
        CaelusWindow(CaelusWindow const &) = delete;
        void Init();
        void Validation();
        void FitToOuter();
        bool m_throwOnUnresolved = true;
        CaelusClassMap m_definedClasses = {};
        bool m_resizable = false;
        HWND m_outerHwnd = 0;
    };

}