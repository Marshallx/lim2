#pragma once

#include "AnusClass.h"
#include "AnusElement.h"

namespace Anus
{
    class AnusWindow : public AnusElement
    {
    public:
        AnusWindow();
        AnusWindow(std::filesystem::path const & file);
        AnusWindow(std::string_view const & source);
        AnusClassMap const & GetClassMap() const;
        int Start(HINSTANCE hInstance, int const nCmdShow);
        void IgnoreErrors(bool const ignore = true);

    private:
        AnusWindow(AnusWindow const &) = delete;
        void SetDefaults();
        bool m_throwOnUnresolved = true;
        AnusClassMap m_definedClasses = {};
    };
}