#include "common.h"


static int nr_hooks = 0;
static gameloop_func_t hooks[MAX_GAMELOOP_HOOKS];

void add_gameloop_hook(gameloop_func_t funcptr)
{
    // this function can be called before init_gameloop_hook()
    if (nr_hooks >= MAX_GAMELOOP_HOOKS) fail("too many gameloop hooks.");
    hooks[nr_hooks++] = funcptr;
}

void call_gameloop_hooks(int flag)
{
    int i;
    for (i = 0; i < nr_hooks; i++) {
        hooks[i](flag);
    }
}

static MAKE_ASMPATCH(gameloop_normal)
{
    call_gameloop_hooks(GAMELOOP_NORMAL);
    RETADDR = 0x541B18;
}
static MAKE_ASMPATCH(gameloop_sleep)
{
    call_gameloop_hooks(GAMELOOP_SLEEP);
    RETADDR = 0x541B18;
}
static MAKE_ASMPATCH(gameloop_movie)
{
    call_gameloop_hooks(GAMELOOP_MOVIE);
    RETADDR = 0x53C62E;
}

void init_gameloop_hook()
{    
    // patch main game loop
    INIT_ASMPATCH(gameloop_normal, 0x541BCD, 5, "\xE9\x46\xFF\xFF\xFF");
    INIT_ASMPATCH(gameloop_sleep, 0x541BB7, 5, "\xE9\x5C\xFF\xFF\xFF");

    // patch movie loop
    SIMPLE_PATCH(0x53C67A, "\xB3", "\x67", 1);
    INIT_ASMPATCH(gameloop_movie, 0x53C6E2, 5, "\x90\x90\x90\x90\x90");
}
