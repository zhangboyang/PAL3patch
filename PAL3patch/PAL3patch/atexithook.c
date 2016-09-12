#include "common.h"

static int nr_atexit_hooks = 0;
static void (*atexit_hooks[MAX_ATEXIT_HOOKS])(void);


void add_atexit_hook(void (*funcptr)(void))
{
    if (nr_atexit_hooks >= MAX_ATEXIT_HOOKS) fail("too many atexit hooks.");
    atexit_hooks[nr_atexit_hooks++] = funcptr;
}

static void run_atexit_hooks()
{
    int i;
    for (i = nr_atexit_hooks - 1; i >= 0; i--) {
        atexit_hooks[i]();
    }
}

static MAKE_ASMPATCH(atexit)
{
    run_atexit_hooks();
    
    RETADDR = POP_DWORD(); // oldcode
    R_ESP += 0x10;
}

void init_atexit_hook()
{
    INIT_ASMPATCH(atexit, 0x00541C35, 5, "\xC2\x10\x00\x90\x90");
}
