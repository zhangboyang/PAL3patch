#include <windows.h>
#include "common.h"
#include <stdio.h>

struct cycle_info_t {
    DWORD lasttime;
};

static struct cycle_info_t cycle_info[2];
static DWORD cycle_ms;

static void try_sleep(struct cycle_info_t *info)
{
    DWORD curtime;
    while (1) {
        curtime = timeGetTime();
        if (curtime >= info->lasttime + cycle_ms) break;
        Sleep(cycle_ms - (curtime - info->lasttime));
    }
    info->lasttime = curtime;
}

static void powersave_hook(int flag)
{
    if (flag == GAMELOOP_NORMAL) {
        try_sleep(&cycle_info[0]);
    } else if (flag == GAMELOOP_MOVIE) {
        try_sleep(&cycle_info[1]);
    }
}

MAKE_PATCHSET(powersave)
{
    cycle_ms = flag;
    memset(cycle_info, 0, sizeof(cycle_info));
    
    add_gameloop_hook(powersave_hook);
}
