#include "common.h"

// kahan compensation
//  no need to reset when setting value to PAL3_m_gametime
//  because the error is very small, can be ignored
static float c = 0.0f;

static MAKE_ASMPATCH(update_gametime)
{
    float deltaTime = M_FLOAT(R_ESP + 0x24);
    
    // use Kahan summation algorithm to do: PAL3_m_gametime += deltaTime;
    float y = deltaTime - c;
    float t = PAL3_m_gametime + y;
    c = (t - PAL3_m_gametime) - y;
    
    // check if we should reset compensation to avoid gametime decrease
    if (t >= PAL3_m_gametime) {
        PAL3_m_gametime = t;
    } else {
        c = 0.0f;
    }
}


MAKE_PATCHSET(kahantimer)
{
    // patch PAL3::Update
    // not using a wrapper because there is already a wrapper in hook.c
    SIMPLE_PATCH_NOP(0x004055D0, "\xD9\x05\xA0\xDA\xBF\x00\xD8\x44\x24\x04", 10);
    INIT_ASMPATCH(update_gametime, 0x004055E4, 6, "\xD9\x1D\xA0\xDA\xBF\x00");
}
