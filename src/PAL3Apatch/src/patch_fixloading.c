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
    INIT_WRAPPER_CALL(UpdateLoading_wrapper, { 0x0041E8A1, 0x0041E9D0, 0x0041EAFD, 0x0041EB9C, 0x0041EBCD });
}
