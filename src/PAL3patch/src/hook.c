#include "common.h"

// the hook framework
static int nr_hooks[MAX_HOOK_TYPES];
static void *hookfunc[MAX_HOOK_TYPES][MAX_HOOKS];

static void add_hook(int hookid, void *funcptr)
{
    if (hookid >= MAX_HOOK_TYPES) fail("invalid hook type %d.", hookid);
    if (nr_hooks[hookid] >= MAX_HOOKS) fail("too many hooks for type %d.", hookid);
    hookfunc[hookid][nr_hooks[hookid]++] = funcptr;
}

static void run_hooks_witharg(int hookid, void *arg)
{
    int i;
    for (i = 0; i < nr_hooks[hookid]; i++) {
        ((void (*)(void *)) hookfunc[hookid][i])(arg);
    }
}
static void run_hooks(int hookid)
{
    int i;
    for (i = 0; i < nr_hooks[hookid]; i++) {
        ((void (*)(void)) hookfunc[hookid][i])();
    }
}
static void run_hooks_reverse_witharg(int hookid, void *arg)
{
    int i;
    for (i = nr_hooks[hookid] - 1; i >= 0; i--) {
        ((void (*)(void *)) hookfunc[hookid][i])(arg);
    }
}
/*static void run_hooks_reverse(int hookid)
{
    int i;
    for (i = nr_hooks[hookid] - 1; i >= 0; i--) {
        ((void (*)(void)) hookfunc[hookid][i])();
    }
}*/



// pre-EndScene and post-Present hooks
void add_preendscene_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_PREENDSCENE, funcptr);
}
void add_postpresent_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_POSTPRESENT, funcptr);
}
void call_preendscene_hooks()
{
    run_hooks(HOOKID_PREENDSCENE);
}
void call_postpresent_hooks()
{
    run_hooks(HOOKID_POSTPRESENT);
}
static MAKE_THISCALL(void, gbGfxManager_D3D_EndScene_wrapper, struct gbGfxManager_D3D *this)
{
    call_preendscene_hooks();
    gbGfxManager_D3D_EndScene(this);
    call_postpresent_hooks();
}
static void init_preendscene_postpresent_hook()
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
static void init_postd3dcreate_hook()
{
    INIT_WRAPPER_CALL(PAL3_InitGFX_wrapper, { 0x00404840 });
}


// post PAL3::Create and pre PAL3::Destroy hooks
void add_postpal3create_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_POSTPAL3CREATE, funcptr);
}
void add_postgamecreate_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_POSTGAMECREATE, funcptr);
}
void add_prepal3destroy_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_PREPAL3DESTROY, funcptr);
}
static int pal3_created = 0;
static void PAL3_Create_wrapper(HINSTANCE hinst)
{
    PAL3_Create(hinst);
    pal3_created = 1;
    run_hooks(HOOKID_POSTPAL3CREATE);
}
static MAKE_ASMPATCH(post_gamecreate)
{
    // this asmpatch shall run just before outputing 'Game Create OK ...' log message
    run_hooks(HOOKID_POSTGAMECREATE);
    PUSH_DWORD(0x005837E4);
}
static void PAL3_Destroy_wrapper()
{
    if (pal3_created) {
        run_hooks(HOOKID_PREPAL3DESTROY);
        pal3_created = 0;
    }
    PAL3_Destroy();
}
static void init_postpal3create_prepal3destroy_hook()
{
    INIT_ASMPATCH(post_gamecreate, 0x004049FC, 5, "\x68\xE4\x37\x58\x00");
    INIT_WRAPPER_CALL(PAL3_Create_wrapper, { 0x00541AC0 });
    INIT_WRAPPER_CALL(PAL3_Destroy_wrapper, {
        0x004047F0,
        0x00541C0B,
    });
}



// game pause and resume hooks
static int pause_state = 0;
void add_pauseresume_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_GAMEPAUSERESUME, funcptr);
}
void set_pauseresume(int state)
{
    state = !!state;
    if (pause_state != state) {
        pause_state = state;
        run_hooks_witharg(HOOKID_GAMEPAUSERESUME, &pause_state);
    }
}
static void PAL3_Update_wrapper(float deltaTime)
{
    set_pauseresume(0);
    PAL3_Update(deltaTime);
}
static void init_pauseresume_hook()
{
    INIT_WRAPPER_CALL(PAL3_Update_wrapper, { 0x00541BC0 });
}



// atexit hook
void call_atexit_hooks()
{
    run_hooks(HOOKID_ATEXIT);
}
static MAKE_ASMPATCH(atexit_normal)
{
    call_atexit_hooks();
    
    LINK_RETN(0x10);
}
void add_atexit_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ATEXIT, funcptr);
}
static void init_atexit_hook()
{
    INIT_ASMPATCH(atexit_normal, 0x00541C35, 5, "\xC2\x10\x00\x90\x90");
}



// gameloop hook
void add_gameloop_hook(void (*funcptr)(void *))
{
    // you need to check gameloop_hookflag in your hook function
    add_hook(HOOKID_GAMELOOP, funcptr);
}
void call_gameloop_hooks(int type, void *data)
{
    struct game_loop_hook_data arg = { .type = type, .data = data };
    run_hooks_witharg(HOOKID_GAMELOOP, &arg);
}
static MAKE_ASMPATCH(gameloop_normal)
{
    set_pauseresume(0);
    call_gameloop_hooks(GAMELOOP_NORMAL, NULL);
    LINK_JMP(0x00541B18);
}
static MAKE_ASMPATCH(gameloop_sleep)
{
    set_pauseresume(1);
    call_gameloop_hooks(GAMELOOP_SLEEP, NULL);
    Sleep(100);
}
static int (*gbBinkVideo_SFLB_OpenFile)(struct gbBinkVideo *, const char *, HWND, int, int) = NULL;
static MAKE_THISCALL(int, gbBinkVideo_OpenFile, struct gbBinkVideo *this, const char *szFileName, HWND hWnd, int bChangeScreenMode, int nOpenFlag)
{
    int ret = gbBinkVideo_SFLB_OpenFile(this, szFileName, hWnd, bChangeScreenMode, nOpenFlag);
    call_gameloop_hooks(GAMEEVENT_MOVIE_ATOPEN, (void *) szFileName);
    return ret;
}
static MAKE_ASMPATCH(gameloop_movie_atenter)
{
    if (g_bink.m_hBink) call_gameloop_hooks(GAMEEVENT_MOVIE_ATBEGIN, NULL);
    R_EDI = R_ECX;
    R_EAX = M_DWORD(R_EDI + 0x0C);
}
static MAKE_ASMPATCH(gameloop_movie)
{
    set_pauseresume(0);
    call_gameloop_hooks(GAMELOOP_MOVIE, NULL);
    LINK_JMP(0x0053C62E);
}
static MAKE_ASMPATCH(gameloop_movie_atexit)
{
    call_gameloop_hooks(GAMEEVENT_MOVIE_ATEND, NULL);
    // we use simple patch to do RETN 4 (the old code)
}
static void init_gameloop_hook()
{    
    // patch main game loop
    INIT_ASMPATCH(gameloop_normal, 0x541BCD, 5, "\xE9\x46\xFF\xFF\xFF");
    INIT_ASMPATCH(gameloop_sleep, 0x541BAF, 8, "\x6A\x64\xFF\x15\x4C\xA0\x56\x00");

    // patch movie loop
    gbBinkVideo_SFLB_OpenFile = TOPTR(get_branch_jtarget(0x0053C455, 0xE8));
    make_jmp(0x0053C440, gbBinkVideo_OpenFile);
    INIT_ASMPATCH(gameloop_movie_atenter, 0x53C5A7, 5, "\x8B\xF9\x8B\x47\x0C");
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
static BOOL WINAPI GetCursorPos_wrapper(LPPOINT lpPoint)
{
    BOOL ret = GetCursorPos(lpPoint);
    if (ret) run_hooks_witharg(HOOKID_GETCURSORPOS, lpPoint);
    return ret;
}
void add_getcursorpos_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_GETCURSORPOS, funcptr);
}
static void init_getcursorpos_hook()
{
    make_jmp(0x004022E0, GetCursorPos_wrapper);
    make_branch(0x00402497, 0xE8, GetCursorPos_wrapper, 6);
    make_branch(0x004021C1, 0xE8, GetCursorPos_wrapper, 6);
    make_branch(0x004D6216, 0xE8, GetCursorPos_wrapper, 6);
    // NOTE: No need to hook GetCursorPos() for IDirect3DDevice9::SetCursorPosition()
    //       see documentation for details.
}

// SetCursorPos hook
static BOOL WINAPI SetCursorPos_wrapper(int X, int Y)
{
    POINT pt = (POINT) { .x = X, .y = Y };
    run_hooks_reverse_witharg(HOOKID_SETCURSORPOS, &pt);
    return SetCursorPos(pt.x, pt.y);
}
void add_setcursorpos_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_SETCURSORPOS, funcptr);
}
static void init_setcursorpos_hook()
{
    make_jmp(0x00402290, SetCursorPos_wrapper);
}


// init all hooks
void init_hooks()
{
    memset(nr_hooks, 0, sizeof(nr_hooks));
    init_gameloop_hook();
    init_atexit_hook();
    init_getcursorpos_hook();
    init_setcursorpos_hook();
    init_postd3dcreate_hook();
    init_postpal3create_prepal3destroy_hook();
    init_preendscene_postpresent_hook();
    init_pauseresume_hook();
}
