#include "common.h"

static MAKE_ASMPATCH(testcombat)
{
    M_BYTE(0x00C01CFC) = 1;
}

MAKE_PATCHSET(testcombat)
{
    INIT_ASMPATCH(testcombat, 0x00406722, 6, "\x88\x1D\xFC\x1C\xC0\x00");
    
    if (!GET_PATCHSET_FLAG(graphicspatch)) {
        SIMPLE_PATCH(0x00408073, "\xC7\x05\xA8\x17\xC0\x00\x01\x00\x00\x00", "\xC7\x05\xA8\x17\xC0\x00\x00\x00\x00\x00", 10);
    }

    // COMCTL32.DLL must be loaded and initialized
    // see notes20160717.txt for details
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    if (!InitCommonControlsEx(&InitCtrls)) {
        fail("InitCommonControlsEx() failed.");
    }
}
