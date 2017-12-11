#include "common.h"

static MAKE_THISCALL(void, UIBigMap_Create_wrapper, struct UIBigMap *this)
{
    UIBigMap_Create(this);
    
    struct uiwnd_ptag ptag;
    
    ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    set_uiwnd_ptag(pUIWND(&this->m_Answer), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_Msg), ptag);
}



// UIBigMap::Update may delete itself
//     Update() -> GoingTo() -> UIGameFrm::SwitchFrm() [delete m_curfrm]
// so we manually maintain fixui state here
static void pre_UIBigMap(struct UIWnd *this)
{
    fixui_pushstate(&game_frect_original_big_lt, &game_frect_ui_auto, TR_SCALE_LOW, TR_SCALE_LOW, ui_big_scalefactor);
}
static void post_UIBigMap(struct UIWnd *this)
{
    fixui_popstate();
}

static MAKE_UIWND_RENDER_WRAPPER_CUSTOM(UIBigMap_Render_wrapper, 0x0044EE44, pre_UIBigMap, post_UIBigMap)
static MAKE_UIWND_UPDATE_WRAPPER_CUSTOM(UIBigMap_Update_wrapper, 0x0044F124, pre_UIBigMap, post_UIBigMap)

MAKE_PATCHSET(fixbigmap)
{
    SIMPLE_PATCH_NOP(0x0044EA81, "\x75\x2B", 2);
    INIT_WRAPPER_CALL(UIBigMap_Create_wrapper, { 0x0045623F });
    INIT_WRAPPER_VFPTR(UIBigMap_Render_wrapper, 0x00558EF8);
    INIT_WRAPPER_VFPTR(UIBigMap_Update_wrapper, 0x00558EFC);
}
