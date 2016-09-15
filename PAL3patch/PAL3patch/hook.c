#include "common.h"

// the hook framework
static int nr_hooks[MAX_HOOK_TYPES] = {};
static void (*hookfunc[MAX_HOOK_TYPES][MAX_HOOKS])(void);

static void add_hook(int hookid, void (*funcptr)(void))
{
    if (hookid >= MAX_HOOK_TYPES) fail("invalid hook type %d.", hookid);
    if (nr_hooks[hookid] >= MAX_HOOKS) fail("too many hooks for type %d.", hookid);
    hookfunc[hookid][nr_hooks[hookid]++] = funcptr;
}

static void run_hooks(int hookid)
{
    int i;
    for (i = nr_hooks[hookid] - 1; i >= 0; i--) {
        hookfunc[hookid][i]();
    }
}





// atexit hook
static MAKE_ASMPATCH(atexit)
{
    run_hooks(HOOKID_ATEXIT);
    
    RETADDR = POP_DWORD(); // oldcode
    R_ESP += 0x10;
}
void add_atexit_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ATEXIT, funcptr);
}
void init_atexit_hook()
{
    INIT_ASMPATCH(atexit, 0x00541C35, 5, "\xC2\x10\x00\x90\x90");
}


// gameloop hook
int gameloop_hookflag;
void add_gameloop_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_GAMELOOP, funcptr);
}
void call_gameloop_hooks(int flag)
{
    gameloop_hookflag = flag;
    run_hooks(HOOKID_GAMELOOP);
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
