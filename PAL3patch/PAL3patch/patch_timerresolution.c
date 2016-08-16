#include <windows.h>
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
    atexit(reset_timer_res); // FIXME: this function seems never called
}
