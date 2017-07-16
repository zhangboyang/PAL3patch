#include "common.h"

static MAKE_ASMPATCH(fix_total_objcnt)
{
    if (!M_BYTE(R_ECX) && M_DWORD(R_ECX - 800)) {
        M_DWORD(R_EBP + 8)++;
    }
}

MAKE_PATCHSET(fixhockshopbuy)
{
    INIT_ASMPATCH(fix_total_objcnt, 0x0051F55D, 5, "\x75\x03\xFF\x45\x08");
}
