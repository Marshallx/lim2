#include "sqlite3/sqlite3.h"
#include "Config.h"
#include "MxUtils.h"

#include "Limb.h"

namespace mx
{
    using namespace Caelus;

    Caelus::Window * Limb::gui = nullptr;
    Database Limb::db = {};

    std::filesystem::path Limb::GetRelPath(char const * relPath)
    {
        return MxUtils::GetModuleFilePath().parent_path() / relPath;
    }

    int Limb::Start(HINSTANCE hInstance, int nCmdShow)
    {
        Config::Load();
        db.Open(GetRelPath("inventory.sqlite"));
        db.GetVersion();
        BuildGui();
        auto const exitcode = gui->start(hInstance, nCmdShow);
        Config::Save();
        return exitcode;
    }

    void Limb::BuildGui()
    {
        gui = new Window(GetRelPath("resource\\search.Caelus"));
        gui->setLabel("Lego Inventory Manager 2");
        //CreateQueryBar();
    }

    void Limb::CreateQueryBar()
    {
        auto bar = gui->addChild("queryBar");
        bar->tether(Edge::LEFT, "outer", Edge::LEFT, 0);
        bar->tether(Edge::RIGHT, "outer", Edge::RIGHT, 0);

        auto searchBox = bar->addChild("searchBox");
        searchBox->setType(ElementType::EDIT);

        //buttonSearch_;
        //buttonAddPart_;
        //editPartsPerPage_;
        //listOrderPartsBy_;
        //cboxExcludeMinifigs_;
        //cboxFindMissingImages_;
        //editLastChecked_;
        //cboxExcludeUnchecked_;
        //cboxHideFulfilled_;
        //cboxHideIgnored_;
        //cboxFindFlagged_;
        //buttonRecalculateWastage_;
        //buttonExportWantedList_;
        //cboxSpecificSets_;
        //editSpecificSets_;
        //cboxShowSetInventoryRecords_;
    }
}