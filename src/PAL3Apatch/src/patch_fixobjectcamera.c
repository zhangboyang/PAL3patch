#include "common.h"

static MAKE_THISCALL(void, ObjectCamera_Create_wrapper, struct ObjectCamera *this, struct gbGfxManager *pMgr)
{
    ObjectCamera_Create(this, pMgr);
    
    // set fov
    float *fov = &this->m_pCamera->fov;
    *fov = atan(tan(*fov * (M_PI / 360.0)) * (game_scalefactor / ui_scalefactor)) / (M_PI / 360.0);
}

MAKE_PATCHSET(fixobjectcamera)
{
    INIT_WRAPPER_CALL(ObjectCamera_Create_wrapper, {
        0x00481493,
        0x00521662,
    });
}
