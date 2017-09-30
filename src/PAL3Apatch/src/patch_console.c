#include "common.h"

static MAKE_ASMPATCH(set_console_enable)
{
    M_DWORD(R_ESI + 0x2218) = 1;
}

MAKE_PATCHSET(console)
{
    INIT_ASMPATCH(set_console_enable, 0x00525906, 6, "\x89\x9E\x18\x22\x00\x00");
}
