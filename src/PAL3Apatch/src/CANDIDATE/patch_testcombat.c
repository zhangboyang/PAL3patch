#include "common.h"

static MAKE_ASMPATCH(testcombat)
{
    M_BYTE(0xBFDA90) = 1;
    R_EAX = M_DWORD(0x5833BC); // old code
}

static BOOL (WINAPI *Real_SetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
static BOOL WINAPI My_SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    X += PAL3_s_drvinfo.width - GAME_WIDTH_ORG;
    Y += PAL3_s_drvinfo.height - GAME_HEIGHT_ORG;
    return Real_SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

MAKE_PATCHSET(testcombat)
{
    INIT_ASMPATCH(testcombat, 0x004047BF, 10, "\xA2\x90\xDA\xBF\x00\xA1\xBC\x33\x58\x00");
    
    if (!GET_PATCHSET_FLAG(graphicspatch)) {
        SIMPLE_PATCH(0x004064DA, "\xC7\x05\xE8\xD6\xBF\x00\x01\x00\x00\x00", "\xC7\x05\xE8\xD6\xBF\x00\x00\x00\x00\x00", 10);
    }

    Real_SetWindowPos = hook_import_table(GetModuleHandle(NULL), "USER32.DLL", "SetWindowPos", My_SetWindowPos);

    // COMCTL32.DLL must be loaded and initialized
    // see notes20160717.txt for details
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    if (!InitCommonControlsEx(&InitCtrls)) {
        fail("InitCommonControlsEx() failed.");
    }
}
