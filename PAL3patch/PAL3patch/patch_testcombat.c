#include <string.h>
#include "common.h"

// for InitCommonControlsEx
#define _WIN32_IE	0x0300
#include <windows.h>
#include <commctrl.h>


MAKE_PATCH(testcombat, 0x004047BF, 10)
{
    MEM_BYTE(0xBFDA90) = 1;
    R_EAX = MEM_DWORD(0x5833BC); // old code
}


MAKE_PATCHSET(testcombat)
{
    INIT_PATCH(testcombat, 0x004047BF, 10, "\xA2\x90\xDA\xBF\x00\xA1\xBC\x33\x58\x00");
    SIMPLE_PATCH(0x004064DA, "\xC7\x05\xE8\xD6\xBF\x00\x01\x00\x00\x00", "\xC7\x05\xE8\xD6\xBF\x00\x00\x00\x00\x00", 10);

    // COMCTL32.DLL must be loaded and initialized
    // see notes20160717.txt for details
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    if (!InitCommonControlsEx(&InitCtrls)) {
        fail("InitCommonControlsEx() failed.");
    }
}
