#include "common.h"

static bool fake_ReadReg(char *subkey, char *val, char *data)
{
    strcpy(data, ".");
    return true;
}

MAKE_PATCHSET(cdpatch)
{
    INIT_WRAPPER_CALL(fake_ReadReg, { 0x0052B96A, 0x00526831 });
    
    // NOTE: space is limited
    strcpy(TOPTR(0x00601B4C), "%s\\%s");    // gbBinkVideo::InitBinkSys
    strcpy(TOPTR(0x006045D4), "%s\\Movie"); // WinMain, sub_5267FE
}
