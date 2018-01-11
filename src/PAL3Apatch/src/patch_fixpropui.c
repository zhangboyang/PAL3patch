#include "common.h"

static MAKE_THISCALL(void, PropUI_Create_wrapper, struct PropUI *this, struct UIWnd *pWnd)
{
    PropUI_Create(this, pWnd);
    
    struct uiwnd_ptag ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    ptag.self_srcrect_type = PTR_GAMERECT_ORIGINAL_LT;
    set_uiwnd_ptag(pUIWND(&this->m_Msg), ptag);
}

MAKE_PATCHSET(fixpropui)
{
    INIT_WRAPPER_CALL(PropUI_Create_wrapper, {
        0x00461D81,
        0x0051C15B,
    });
}
