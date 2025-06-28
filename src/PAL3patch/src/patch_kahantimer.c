#include "common.h"

static float zero_gametime = 0.0f;
static float last_gametime = 0.0f;
static double c = 0.0; // kahan compensation

static MAKE_ASMPATCH(update_gametime)
{
    float deltaTime = M_FLOAT(R_ESP + 0x24);
    
    if (memcmp(&PAL3_m_gametime, &zero_gametime, sizeof(float)) == 0 || memcmp(&PAL3_m_gametime, &last_gametime, sizeof(float)) != 0) {
        c = 0.0;
    }
    
    // use Kahan summation algorithm to do: PAL3_m_gametime += deltaTime;
    double y = deltaTime - c;
    float t = PAL3_m_gametime + y;
    c = (t - PAL3_m_gametime) - y;
    PAL3_m_gametime = t;
    
    memcpy(&last_gametime, &PAL3_m_gametime, sizeof(float));
}


MAKE_PATCHSET(kahantimer)
{
    // patch PAL3::Update
    // not using a wrapper because there is already a wrapper in hook.c
    SIMPLE_PATCH_NOP(0x004055D0, "\xD9\x05\xA0\xDA\xBF\x00\xD8\x44\x24\x04", 10);
    INIT_ASMPATCH(update_gametime, 0x004055E4, 6, "\xD9\x1D\xA0\xDA\xBF\x00");
}
