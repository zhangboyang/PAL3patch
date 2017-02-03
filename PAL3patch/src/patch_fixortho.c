#include "common.h"

// orthofactor is used outside this file
// fixortho might not be initialized
// must use static initialize method
float orthofactor = 1.0;


static float OrthoSize_sv;
static int sv_level = 0;

static void wrapper_pre_hook(struct gbCamera *this)
{
    if (this->fov == 0.0f) {
        if (sv_level == 0) {
            OrthoSize_sv = this->OrthoSize;
            this->OrthoSize *= orthofactor;
        } else {
            warning("gbcamera hook re-entered.");
        }
        sv_level++;
    }
}
static void wrapper_post_hook(struct gbCamera *this)
{
    if (this->fov == 0.0f) {
        sv_level--;
        if (sv_level == 0) {
            this->OrthoSize = OrthoSize_sv;
        }
    }
}



static MAKE_THISCALL(void, gbCamera_SetAsCurrent_wrapper, struct gbCamera *this, int a2)
{
    wrapper_pre_hook(this);
    gbCamera_SetAsCurrent(this, a2);
    wrapper_post_hook(this);
}
static MAKE_THISCALL(void, gbCamera_PointEyeToScr_100220B0_wrapper, struct gbCamera *this, struct gbVec3D *a2, float *a3, float *a4)
{
    wrapper_pre_hook(this);
    gbCamera_PointEyeToScr_100220B0(this, a2, a3, a4);
    wrapper_post_hook(this);
}
static MAKE_THISCALL(void, gbCamera_GetRayToScreen_wrapper, struct gbCamera *this, float a2, float a3, struct gbRay *a4)
{
    wrapper_pre_hook(this);
    gbCamera_GetRayToScreen(this, a2, a3, a4);
    wrapper_post_hook(this);
}
static void init_gbcamera_wrappers()
{
    /*
        wrapper summary
        the address is taken from patch_fixfov.c
        Y/N means if wrapper is needed
        
        N 0x10021A2B GetViewSizeOnNearPlane // only called in GetRayToScreen, but ortho is in another exec flow, dllexport not used
        Y 0x10021AA5 SetAsCurrent // no xref, dllexport IS used
        N 0x10021FD8 PointScrToWorld // no xref, dllexport not used
        N 0x10022068 PointScrToEye // no xref, dllexport not used
        Y 0x100220F5 PointEyeToScr_100220B0 // xref IS used, dllexport not used
        N 0x100221B5 PointEyeToScr_10022170 // no xref, dllexport not used
        
        Y 0x100222C0 GetRayToScreen // no xref, dllexport IS used
    */
    INIT_WRAPPER_VFPTR(gbCamera_SetAsCurrent_wrapper, 0x0056A2DC); // dllexport
    INIT_WRAPPER_VFPTR(gbCamera_GetRayToScreen_wrapper, 0x0056A2EC); // dllexport
    INIT_WRAPPER_CALL(gbCamera_PointEyeToScr_100220B0_wrapper, { 0x100222B4, 0x100226CD }); // xref
}

MAKE_PATCHSET(fixortho)
{
    init_gbcamera_wrappers();
    //orthofactor = str2double(get_string_from_configfile("orthofactor"));
    if (game_width * 3 > game_height * 4) {
        orthofactor *= game_width * 0.75f / game_height;
    }
}
