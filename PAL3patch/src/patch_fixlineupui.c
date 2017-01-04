#include "common.h"

static MAKE_UIWND_RENDER_WRAPPER(LineupUI_Render_wrapper, 0x004850E0)
static MAKE_UIWND_UPDATE_WRAPPER(LineupUI_Update_wrapper, 0x00485230)

static void __fastcall LineupUI_Create_wrapper(struct LineupUI *this, int dummy, struct UIWnd *pWnd)
{
    LineupUI_Create(this, pWnd);
    
    // set ptag
    struct uiwnd_ptag ptag = MAKE_PTAG(SF_UI, PTR_GAMERECT_ORIGINAL, PTR_GAMERECT_UIAUTO, TR_SCALE_LOW, TR_SCALE_LOW); // same as FIXUI_AUTO_TRANSFORM
    ptag.no_cursor_virt = 1;
    ptag.self_only_ptag = 1;
    set_uiwnd_ptag((struct UIWnd *) this, ptag);
    
    // set fov
    float *fov = &this->m_pCamera->m_pCamera->fov;
    *fov = atan(tan(*fov * (M_PI / 360.0)) * (game_scalefactor / ui_scalefactor)) / (M_PI / 360.0);
}

MAKE_PATCHSET(fixlineupui)
{
    INIT_WRAPPER_VFPTR(LineupUI_Render_wrapper, 0x0056BA34);
    INIT_WRAPPER_VFPTR(LineupUI_Update_wrapper, 0x0056BA38);
    INIT_WRAPPER_CALL(LineupUI_Create_wrapper, { 0x004A46F8 });
}
