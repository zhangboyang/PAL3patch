#include "common.h"

static MAKE_UIWND_RENDER_WRAPPER(LineupComp_Render_wrapper, 0x00521F3F)
static MAKE_UIWND_UPDATE_WRAPPER(LineupComp_Update_wrapper, 0x00521CA0)

static MAKE_THISCALL(void, LineupComp_Create_wrapper, struct LineupComp *this)
{
    LineupComp_Create(this);
    
    // set ptag
    struct uiwnd_ptag ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    ptag.no_cursor_virt = 1;
    set_uiwnd_ptag(pUIWND(this), ptag);
}

MAKE_PATCHSET(fixcompdonateui)
{
    INIT_WRAPPER_VFPTR(LineupComp_Render_wrapper, 0x0055EF9C);
    INIT_WRAPPER_VFPTR(LineupComp_Update_wrapper, 0x0055EFA0);
    INIT_WRAPPER_VFPTR(LineupComp_Create_wrapper, 0x0055EFB4);
    
    // patch 1024x768 check
    SIMPLE_PATCH_NOP(0x00521676, "\x75\x08", 2);
}
