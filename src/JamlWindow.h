#pragma once

#include "JamlClass.h"
#include "JamlElement.h"

namespace jaml
{
    class JamlWindow : public JamlElement
    {
    public:
        JamlWindow();
        JamlWindow(std::filesystem::path const & file);
        JamlWindow(std::string_view const & source);
        JamlClassMap const & GetClassMap() const;
        int Start(HINSTANCE hInstance, int const nCmdShow);
        void IgnoreErrors(bool const ignore = true);

    private:
        JamlWindow(JamlWindow const &) = delete;
        void SetDefaults();
        bool m_throwOnUnresolved = true;
        JamlClassMap m_definedClasses = {};
    };
}