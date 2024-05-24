#include "sqlite3/sqlite3.h"
#include "Config.h"
#include "LimbGui.h"
#include "MxUtils.h"

#include "Limb.h"

namespace mx
{
    using namespace jaml;

    jaml::Window Limb::gui;
    Database Limb::db = {};

    int Limb::Start(HINSTANCE hInstance, int nCmdShow)
    {
        Config::Load();
        db.Open(MxUtils::GetModuleFilePath().replace_filename("inventory.sqlite"));
        db.GetVersion();
        BuildGui();
        auto const exitcode = gui.start(hInstance, nCmdShow);
        Config::Save();
        return exitcode;
    }

    void Limb::BuildGui()
    {
        gui.setLabel("Lego Inventory Manager 2");
        CreateQueryBar();
    }

    void Limb::CreateQueryBar()
    {
        auto bar = gui.addChild("queryBar");
        bar->tether(Side::LEFT, "outer", Side::LEFT, 0);
        bar->tether(Side::RIGHT, "outer", Side::RIGHT, 0);

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