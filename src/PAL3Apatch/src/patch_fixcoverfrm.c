#include "common.h"

static MAKE_THISCALL(void, UICoverFrm_Create_wrapper_forBig, struct UICoverFrm *this)
{
    UICoverFrm_Create(this);
    
    int i;
    struct uiwnd_ptag ptag;
    
    ptag = MAKE_PTAG(SF_UI_BIG, PTR_GAMERECT_ORIGINAL_BIG_LT, PTR_GAMERECT_UIAUTO, TR_SCALE_LOW, TR_SCALE_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_frame), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_tjA), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_tjB), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_XDGS), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_QCYM), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_XJTM), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_LKYX), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_YXXX), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_JDHY), ptag);
    for (i = 0; i < 13; i++) {
        set_uiwnd_ptag(pUIWND(&this->m_Star[i]), ptag);
    }
    set_uiwnd_ptag(pUIWND(&this->m_Meteor), ptag);
    
    ptag = MAKE_PTAG(SF_UI_BIG, PTR_GAMERECT_ORIGINAL_BIG, PTR_GAMERECT_UIAUTO, TR_SCALE_LOW, TR_SCALE_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_background), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_mountainA), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_mountainB), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_Logo), ptag);
}

MAKE_PATCHSET(fixcoverfrm)
{
    if (flag != 2) {  // use UICoverFrm for 800x600
    
        // patch 1024x768 check
        SIMPLE_PATCH(0x004515C8, "\x75\x07", "\xEB\x07", 2);
        
    } else {          // use UICoverFrm for 1024x768
        
        // patch 1024x768 check
        SIMPLE_PATCH_NOP(0x004515C8, "\x75\x07", 2);
        
        // hook UICoverFrm::Create
        INIT_WRAPPER_CALL(UICoverFrm_Create_wrapper_forBig, { 0x00456094 });
    }
}
