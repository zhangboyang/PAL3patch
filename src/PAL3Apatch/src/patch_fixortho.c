#include "common.h"

// orthofactor is used outside this file
// fixortho might not be initialized
// must use static initialize method
float orthofactor = 1.0;


static MAKE_THISCALL(void, gbCamera_GetViewSizeOnNearPlane_wrapper, struct gbCamera *this, float *hw, float *hh)
{
    float OrthoSize_sv;
    OrthoSize_sv = this->OrthoSize;
    this->OrthoSize *= orthofactor;
    gbCamera_GetViewSizeOnNearPlane(this, hw, hh);
    this->OrthoSize = OrthoSize_sv;
}


static void init_gbcamera_wrappers()
{
    // xref, no need to patch export table because PAL3A.exe do not import it
    INIT_WRAPPER_CALL(gbCamera_GetViewSizeOnNearPlane_wrapper, {
        gboffset + 0x10021504,
        gboffset + 0x100219E3,
        gboffset + 0x10021A97,
        gboffset + 0x10021B72,
    });
}

MAKE_PATCHSET(fixortho)
{
    init_gbcamera_wrappers();
    //orthofactor = str2double(get_string_from_configfile("orthofactor"));
    if (game_width * 3 > game_height * 4) {
        orthofactor *= game_width * 0.75f / game_height;
    }
}
