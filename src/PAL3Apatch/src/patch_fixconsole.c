#include "common.h"

static MAKE_THISCALL(void, Console_Render_wrapper, struct Console *this)
{
    if (PAL3_s_gamestate == GAME_UI) {
        fixui_pushstate(&game_frect_original_lt, &game_frect_ui_auto, TR_SCALE_LOW, TR_SCALE_LOW, ui_scalefactor);
        Console_Render(this);
        fixui_popstate();
    } else {
        Console_Render(this);
    }
}

static MAKE_ASMPATCH(fixver)
{
    PUSH_DWORD(GAME_HEIGHT_ORG - 40);
    PUSH_DWORD(GAME_WIDTH_ORG - 160);
}

MAKE_PATCHSET(fixconsole)
{
    SIMPLE_PATCH_NOP(0x00525B44, "\x89\x86\x0C\x22\x00\x00\xC7\x86\x10\x22\x00\x00\x2C\x01\x00\x00", 16);
    INIT_WRAPPER_CALL(Console_Render_wrapper, { 0x00407C3C, 0x004EDC80 });
    INIT_ASMPATCH(fixver, 0x00525AF2, 12, "\x50\xA1\x90\x17\xC0\x00\x05\x60\xFF\xFF\xFF\x50");
}
