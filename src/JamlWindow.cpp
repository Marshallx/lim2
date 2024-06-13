#include <filesystem>
#include <fstream>
#include <iostream>

#include "JamlParser.h"

#include "JamlWindow.h"

namespace jaml
{
    void JamlWindow::defaultFont()
    {
        fontFace = "Arial";
        fontWeight = REGULAR;
        fontStyle = NORMAL;
        fontSize = { 12, PT };
    }

    JamlWindow::JamlWindow()
    {
        defaultFont();
    }

    JamlWindow::JamlWindow(std::string_view const & jamlSource)
    {
        defaultFont();
        JamlParser parser(jamlSource, this);
    }

    JamlWindow::JamlWindow(std::filesystem::path const & file)
    {
        FILE * f = fopen(file.string().c_str(), "r");

        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        auto jamlSource = std::string{};
        jamlSource.resize(size);

        rewind(f);
        fread(jamlSource.data(), sizeof(char), size, f);

        defaultFont();
        JamlParser parser({ jamlSource }, this);
    }

    void JamlWindow::setForceResolve(bool const force)
    {
        throwOnUnresolved = !force;
    }
}