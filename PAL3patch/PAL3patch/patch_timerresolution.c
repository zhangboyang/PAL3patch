#include "common.h"

static int timer_res;
static void reset_timer_res()
{
    timeEndPeriod(timer_res);
}

MAKE_PATCHSET(timerresolution)
{
    timer_res = flag;
    timeBeginPeriod(timer_res);
    add_atexit_hook(reset_timer_res);
}
