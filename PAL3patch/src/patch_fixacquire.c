#include "common.h"

#define ACQUIRE_RETRY_DELAY 100

static MAKE_THISCALL(void, GRPinput_AcquireMouse_wrapper, struct GRPinput *this)
{
    while (1) {
        GRPinput_AcquireMouse(this);
        if (this->m_bMouse) break;
        Sleep(ACQUIRE_RETRY_DELAY);
    }
}

static MAKE_THISCALL(void, GRPinput_AcquireKeyboard_wrapper, struct GRPinput *this)
{
    while (1) {
        GRPinput_AcquireKeyboard(this);
        if (this->m_bKeyboard) break;
        Sleep(ACQUIRE_RETRY_DELAY);
    }
}

MAKE_PATCHSET(fixacquire)
{
    // fix calls in GRPinput::Create only
    // make sure GRPinput::Create won't fail becauseof Acquire()
    INIT_WRAPPER_CALL(GRPinput_AcquireMouse_wrapper, { 0x00402150 });
    INIT_WRAPPER_CALL(GRPinput_AcquireKeyboard_wrapper, { 0x00402198 });
}
