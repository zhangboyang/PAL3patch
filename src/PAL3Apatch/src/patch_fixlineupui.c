#include "common.h"

static MAKE_UIWND_RENDER_WRAPPER(LineupUI_Render_wrapper, 0x00482861)
static MAKE_UIWND_UPDATE_WRAPPER(LineupUI_Update_wrapper, 0x00482994)

static MAKE_THISCALL(void, LineupUI_Create_wrapper, struct LineupUI *this, struct UIWnd *pWnd)
{
    LineupUI_Create(this, pWnd);
    
    // set ptag
    struct uiwnd_ptag ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    ptag.no_cursor_virt = 1;
    ptag.self_only_ptag = 1;
    set_uiwnd_ptag(pUIWND(this), ptag);
}

MAKE_PATCHSET(fixlineupui)
{
    INIT_WRAPPER_VFPTR(LineupUI_Render_wrapper, 0x005599E0);
    INIT_WRAPPER_VFPTR(LineupUI_Update_wrapper, 0x005599E4);
    INIT_WRAPPER_CALL(LineupUI_Create_wrapper, { 0x0049E6F5 });
    
    // patch 1024x768 check
    SIMPLE_PATCH_NOP(0x004814A7, "\x75\x08", 2);
}
