#include <filesystem>
#include <fstream>
#include <iostream>

#include "JamlParser.h"

#include "JamlWindow.h"

namespace jaml
{
    void JamlWindow::SetDefaults()
    {
        auto cp = std::make_shared<JamlClass>();
        definedClasses.push_back(cp);
        auto c = cp.get();
        c->font = {};
        auto f = c->font.value();
        f.SetFace("Arial");
        f.SetSize("12pt");
        f.SetColor(0);
        f.SetWeight(REGULAR);
        f.SetStyle("normal");

        c->SetBackgroundColor(0xFFFFFF);
    }

    JamlWindow::JamlWindow()
    {
        SetDefaults();
    }

    JamlWindow::JamlWindow(std::string_view const & jamlSource)
    {
        SetDefaults();
        JamlParser parser(jamlSource, definedClasses);
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

        SetDefaults();
        JamlParser parser({ jamlSource }, definedClasses);
    }

    void JamlWindow::IgnoreErrors(bool const ignore)
    {
        throwOnUnresolved = !ignore;
    }
}