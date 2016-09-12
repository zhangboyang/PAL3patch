#include "common.h"

static MAKE_ASMPATCH(fixloadingfrm)
{
    RECT *rect = TOPTR(R_ESP + 0x18);
    set_rect(rect, 206, 520, 570, 548);
    transform_rect(rect, TR_CENTER, TR_SHIFTLOWSCALE, TR_NONE);
}

MAKE_PATCHSET(fixloadingfrm)
{
    INIT_ASMPATCH(fixloadingfrm, 0x00450832, 0x20, "\xC7\x44\x24\x1C\x08\x02\x00\x00\xC7\x44\x24\x18\xCE\x00\x00\x00\xC7\x44\x24\x20\x3A\x02\x00\x00\xC7\x44\x24\x24\x24\x02\x00\x00");
}
