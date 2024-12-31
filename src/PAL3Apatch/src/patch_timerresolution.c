#include "common.h"

static int timer_res;
static void reset_timer_res()
{
    timeEndPeriod(timer_res);
}

MAKE_PATCHSET(timerresolution)
{
    timer_res = flag;
    
    TIMECAPS tc;
    if (timeGetDevCaps(&tc, sizeof(tc)) == TIMERR_NOERROR) {
        timer_res = imax(timer_res, tc.wPeriodMin);
        timer_res = imin(timer_res, tc.wPeriodMax);
    }
    
    if (timeBeginPeriod(timer_res) == TIMERR_NOERROR) {
        add_atexit_hook(reset_timer_res);
    }
}
