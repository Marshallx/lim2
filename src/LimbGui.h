#pragma once

#include <Windows.h>

namespace mx
{
    class LimbGui
    {
    public:
        enum PartColumns
        {
            IMAGE, ID, DESC, SETQTY, SETQTYP, BUILT, EXPECT, ACTUAL, SPARE, OWNED, WANTED, BUY, CHECKED, NOTE, FLAGS, ACTIONS, MAX
        };

        void Init(HINSTANCE hInstance, int nCmdShow);
        int Start();
        static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
        static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
        void CreateQueryBar();
        int Em2Px(int em);
        void test();

        //static LRESULT CALLBACK ActionBarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

        HINSTANCE GetHinst() const noexcept;
        HWND GetHwnd() const noexcept;

    private:
        static constexpr auto const MAX_LOADSTRING = int{100};
        HINSTANCE hInst_;
        WCHAR title_[MAX_LOADSTRING];
        WCHAR windowClass_[MAX_LOADSTRING];
        int nCmdShow_;
        HWND hwnd_;

        HWND queryBar_;
        HWND editSearch_;
        HWND buttonSearch_;
        HWND buttonAddPart_;
        HWND editPartsPerPage_;
        HWND listOrderPartsBy_;
        HWND cboxExcludeMinifigs_;
        HWND cboxFindMissingImages_;
        HWND editLastChecked_;
        HWND cboxExcludeUnchecked_;
        HWND cboxHideFulfilled_;
        HWND cboxHideIgnored_;
        HWND cboxFindFlagged_;
        HWND buttonRecalculateWastage_;
        HWND buttonExportWantedList_;
        HWND cboxSpecificSets_;
        HWND editSpecificSets_;
        HWND cboxShowSetInventoryRecords_;

        HWND navBar_;
        HWND buttonFirst_;
        HWND buttonBack_;
        HWND buttonNext_;
        HWND buttonLast_;
        HWND listPage_;

        HWND labelSearch_;

        HWND resultsTable_;
        HWND columnHeaders[PartColumns::MAX];
        
    };
}
