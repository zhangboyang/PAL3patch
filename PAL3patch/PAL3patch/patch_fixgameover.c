#include "common.h"

static MAKE_ASMPATCH(fixgameover)
{
    RECT *rect = TOPTR(R_ESP + 0x18);
    set_rect(rect, 0, 0, 256, 600);
    transform_rect(rect, TR_CENTER, TR_CENTER, TR_NONE);
    adjust_rect(rect, 0, 1024, TR_NONE, TR_HIGH);
}

MAKE_PATCHSET(fixgameover)
{
    INIT_ASMPATCH(fixgameover, 0x00450167, 0x8, "\xC7\x44\x24\x1C\x00\x00\x00\x00");
}
