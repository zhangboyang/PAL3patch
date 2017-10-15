#include "common.h"

#define ACQUIRE_RETRY_DELAY 100

static int do_events()
{
    MSG msg;
    // process message queue here
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage(msg.wParam);
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 1;
}

static MAKE_THISCALL(void, GRPinput_AcquireMouse_wrapper, struct GRPinput *this)
{
    while (1) {
        GRPinput_AcquireMouse(this);
        if (this->m_bMouse) break;
        Sleep(ACQUIRE_RETRY_DELAY);
        if (!do_events()) break;
    }
}

static MAKE_THISCALL(void, GRPinput_AcquireKeyboard_wrapper, struct GRPinput *this)
{
    while (1) {
        GRPinput_AcquireKeyboard(this);
        if (this->m_bKeyboard) break;
        Sleep(ACQUIRE_RETRY_DELAY);
        if (!do_events()) break;
    }
}

MAKE_PATCHSET(fixacquire)
{
    // make sure GRPinput::Create won't fail becauseof Acquire()
    // NOTE: fix calls in GRPinput::Create ONLY
    INIT_WRAPPER_CALL(GRPinput_AcquireMouse_wrapper, { 0x004029AA });
    INIT_WRAPPER_CALL(GRPinput_AcquireKeyboard_wrapper, { 0x004029E9 });
}
