#pragma once

#include <Windows.h>

#include "Database.h"
#include "jaml.h"

namespace mx
{
    class Limb
    {
    public:
        enum PartColumns
        {
            IMAGE, ID, DESC, SETQTY, SETQTYP, BUILT, EXPECT, ACTUAL, SPARE, OWNED, WANTED, BUY, CHECKED, NOTE, FLAGS, ACTIONS, MAX
        };

        static std::filesystem::path GetRelPath(char const * relPath);
        static int Start(HINSTANCE hInstance, int nCmdShow);

    private:
        static void BuildGui();
        static void CreateQueryBar();
        static jaml::Window * gui;
        static Database db;
    };
}