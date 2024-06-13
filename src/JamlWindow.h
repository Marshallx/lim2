#pragma once

#include "JamlElement.h"

namespace jaml
{
    class JamlWindow : public JamlElement
    {
    public:
        JamlWindow();
        JamlWindow(std::filesystem::path const & file);
        JamlWindow(std::string_view const & source);
        int start(HINSTANCE hInstance, int const nCmdShow);
        void setForceResolve(bool const force = true);

    private:
        void defaultFont();
        JamlWindow(JamlWindow const &) = delete;
        bool throwOnUnresolved = true;
    };
}