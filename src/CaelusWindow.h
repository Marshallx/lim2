#pragma once

#include "CaelusClass.h"
#include "CaelusElement.h"

namespace Caelus
{
    class CaelusWindow : public CaelusElement
    {
    public:
        CaelusWindow();
        CaelusWindow(std::filesystem::path const & file);
        CaelusWindow(std::string_view const & source);
        CaelusClassMap const & GetClassMap() const;
        int Start(HINSTANCE hInstance, int const nCmdShow);
        void IgnoreErrors(bool const ignore = true);

    private:
        CaelusWindow(CaelusWindow const &) = delete;
        void SetDefaults();
        bool m_throwOnUnresolved = true;
        CaelusClassMap m_definedClasses = {};
    };
}