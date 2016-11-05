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
    // run hooks by add order
    for (i = 0; i < nr_hooks[hookid]; i++) {
        hookfunc[hookid][i]();
    }
}



// EndScene hooks
void add_preendscene_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_PREENDSCENE, funcptr);
}
void call_preendscene_hooks()
{
    run_hooks(HOOKID_PREENDSCENE);
}
static void __fastcall gbGfxManager_D3D_EndScene_wrapper(struct gbGfxManager_D3D *this, int dummy)
{
    call_preendscene_hooks();
    gbGfxManager_D3D_EndScene(this);
}
void init_preendscene_hook()
{
    INIT_WRAPPER_VFPTR(gbGfxManager_D3D_EndScene_wrapper, gboffset + 0x100F56E8);
}



// OnLostDevice and OnResetDevice hooks
void add_onlostdevice_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ONLOSTDEVICE, funcptr);
}
void call_onlostdevice_hooks()
{
    run_hooks(HOOKID_ONLOSTDEVICE);
}
void add_onresetdevice_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ONRESETDEVICE, funcptr);
}
void call_onresetdevice_hooks()
{
    run_hooks(HOOKID_ONRESETDEVICE);
}


// post d3d create hooks
void add_postd3dcreate_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_POSTD3DCREATE, funcptr);
}
static void PAL3_InitGFX_wrapper()
{
    PAL3_InitGFX();
    run_hooks(HOOKID_POSTD3DCREATE);
}
void init_postd3dcreate_hook()
{
    INIT_WRAPPER_CALL(PAL3_InitGFX_wrapper, { 0x00404840 });
}




// atexit hook
static MAKE_ASMPATCH(atexit)
{
    run_hooks(HOOKID_ATEXIT);
    
    LINK_RETN(0x10);
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
    // you need to check gameloop_hookflag in your hook function
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
    LINK_JMP(0x00541B18);
}
static MAKE_ASMPATCH(gameloop_sleep)
{
    call_gameloop_hooks(GAMELOOP_SLEEP);
    LINK_JMP(0x00541B18);
}
static MAKE_ASMPATCH(gameloop_movie)
{
    call_gameloop_hooks(GAMELOOP_MOVIE);
    LINK_JMP(0x0053C62E);
}
static MAKE_ASMPATCH(gameloop_movie_atexit)
{
    call_gameloop_hooks(GAMELOOP_MOVIE_ATEXIT);
    // we use simple patch to do RETN 4 (the old code)
}
void init_gameloop_hook()
{    
    // patch main game loop
    INIT_ASMPATCH(gameloop_normal, 0x541BCD, 5, "\xE9\x46\xFF\xFF\xFF");
    INIT_ASMPATCH(gameloop_sleep, 0x541BB7, 5, "\xE9\x5C\xFF\xFF\xFF");

    // patch movie loop
    SIMPLE_PATCH(0x53C67A, "\xB3", "\x67", 1);
    SIMPLE_PATCH(0x53C687, "\xA6", "\x5A", 1);
    INIT_ASMPATCH(gameloop_movie, 0x53C6E2, 5, "\x90\x90\x90\x90\x90");
    // patch movie loop atexit
    SIMPLE_PATCH(0x53C6B6, "\xC2\x04\x00", "\xEB\x2F\x90", 3); // first RETN 4
    SIMPLE_PATCH(0x53C6DF, "\xC2\x04\x00", "\xEB\x06\x90", 3); // second RETN 4
    INIT_ASMPATCH(gameloop_movie_atexit, 0x53C6E7, 5, "\x90\x90\x90\x90\x90");
    SIMPLE_PATCH(0x53C6EC, "\x90\x90\x90", "\xC2\x04\x00", 3); // place RETN 4 after ASMPATCH
}



// GetCursorPos hook
LPPOINT getcursorpos_hook_lppoint;
BOOL getcursorpos_hook_ret;
static BOOL WINAPI GetCursorPos_wrapper(LPPOINT lpPoint)
{
    getcursorpos_hook_ret = GetCursorPos(lpPoint);
    getcursorpos_hook_lppoint = lpPoint;
    run_hooks(HOOKID_GETCURSORPOS);
    return getcursorpos_hook_ret;
}
void add_getcursorpos_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_GETCURSORPOS, funcptr);
}
void init_getcursorpos_hook()
{
    make_jmp(0x004022E0, GetCursorPos_wrapper);
    make_branch(0x00402497, 0xE8, GetCursorPos_wrapper, 6);
    make_branch(0x004021C1, 0xE8, GetCursorPos_wrapper, 6);
    make_branch(0x004D6216, 0xE8, GetCursorPos_wrapper, 6);
    // NOTE: No need to hook GetCursorPos() for IDirect3DDevice9::SetCursorPosition()
    //       see documentation for details.
}
