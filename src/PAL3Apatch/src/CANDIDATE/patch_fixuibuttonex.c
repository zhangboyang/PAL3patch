#include "common.h"

static MAKE_ASMPATCH(fix_getpixelchannel)
{
    struct UIWnd *this = TOPTR(R_ESI);
    int y = M_DWORD(R_ESP + 0x8);
    y = (this->m_rect.bottom - this->m_rect.top) - y;
    M_DWORD(R_ESP + 0x8) = y;
    LINK_CALL(0x00445F80);
}

MAKE_PATCHSET(fixuibuttonex)
{
    INIT_ASMPATCH(fix_getpixelchannel, 0x0043B353, 5, "\xE8\x28\xAC\x00\x00");
    INIT_ASMPATCH(fix_getpixelchannel, 0x0043BA72, 5, "\xE8\x09\xA5\x00\x00");
}
