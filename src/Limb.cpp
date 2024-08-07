#include "sqlite3/sqlite3.h"
#include "Config.h"
#include "MxiUtils.h"

#include "Caelus.h"

#include "Limb.h"

namespace mx
{
    using namespace Caelus;

    CaelusWindow * Limb::gui = nullptr;
    Database Limb::db = {};

    std::filesystem::path Limb::GetRelPath(char const * relPath)
    {
        return mxi::GetModuleFilePath().parent_path() / relPath;
    }

    int Limb::Start(HINSTANCE hInstance, int nCmdShow)
    {
        Config::Load();
        db.Open(GetRelPath("inventory.sqlite"));
        db.GetVersion();
        Caelus::Init(hInstance);
        BuildGui();
        auto const exitcode = gui->Start(hInstance, nCmdShow);
        Config::Save();
        return exitcode;
    }

    void Limb::BuildGui()
    {
        gui = new CaelusWindow(GetRelPath("resource\\search.anus"));
        gui->SetLabel("Lego Inventory Manager 2");
    }
}