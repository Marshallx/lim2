#pragma once

#include "CaelusClass.h"

namespace Caelus
{
    class CaelusElement;

    class CaelusWindow
    {
    public:
        CaelusWindow();
        CaelusWindow(std::filesystem::path const & file);
        CaelusWindow(std::string_view const & source);
        CaelusClassMap const & GetClassMap() const;
        int Start(HINSTANCE hInstance, int const nCmdShow);
        void IgnoreErrors(bool const ignore = true);
        void SetTitle(std::string_view const & title);

    private:
        CaelusWindow(CaelusWindow const &) = delete;
        void SetDefaults();
        bool m_throwOnUnresolved = true;
        CaelusClassMap m_definedClasses = {};
        CaelusElement * m_client = nullptr;
        HWND m_hwnd;
    };
}