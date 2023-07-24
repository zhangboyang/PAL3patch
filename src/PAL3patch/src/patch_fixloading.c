#include "common.h"

static DWORD last_update, pause_update;
static void UpdateLoading_wrapper()
{
    if (g_lastupdateloading == 0) {
        pause_update = 0;
        UpdateLoading();
        return;
    }
    
    DWORD t1 = timeGetTime();
    if (t1 - last_update < pause_update) {
        return;
    }
    
    UpdateLoading();
    
    DWORD t2 = timeGetTime();
    last_update = t2;
    pause_update = (t2 - t1) / 1;
}

MAKE_PATCHSET(fixloading)
{
    INIT_WRAPPER_CALL(UpdateLoading_wrapper, { 0x0041FA84, 0x0041FB0A, 0x0041FC35, 0x0041FCE1, 0x0041FD19 });
}
