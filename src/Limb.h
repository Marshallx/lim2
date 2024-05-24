#pragma once

#include <Windows.h>

#include "Database.h"
#include "jaml.h"
#include "LimbGui.h"

namespace mx
{
    class Limb
    {
    public:
        static int Start(HINSTANCE hInstance, int nCmdShow);

    private:
        static void BuildGui();
        static void CreateQueryBar();
        static jaml::Window gui;
        static Database db;
    };
}