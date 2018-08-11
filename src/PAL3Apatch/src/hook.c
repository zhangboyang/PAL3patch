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

static void run_hooks_witharg(int hookid, void *arg, int (*brkcond)(void *arg))
{
    int i;
    for (i = 0; i < nr_hooks[hookid]; i++) {
        ((void (*)(void *)) hookfunc[hookid][i])(arg);
        if (brkcond && brkcond(arg)) break;
    }
}
static void run_hooks(int hookid, int (*brkcond)(void))
{
    int i;
    for (i = 0; i < nr_hooks[hookid]; i++) {
        ((void (*)(void)) hookfunc[hookid][i])();
        if (brkcond && brkcond()) break;
    }
}
static void run_hooks_reverse_witharg(int hookid, void *arg, int (*brkcond)(void *arg))
{
    int i;
    for (i = nr_hooks[hookid] - 1; i >= 0; i--) {
        ((void (*)(void *)) hookfunc[hookid][i])(arg);
        if (brkcond && brkcond(arg)) break;
    }
}
/*static void run_hooks_reverse(int hookid, int (*brkcond)(void))
{
    int i;
    for (i = nr_hooks[hookid] - 1; i >= 0; i--) {
        ((void (*)(void)) hookfunc[hookid][i])();
        if (brkcond && brkcond()) break;
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
    run_hooks(HOOKID_PREENDSCENE, NULL);
}
void call_postpresent_hooks()
{
    run_hooks(HOOKID_POSTPRESENT, NULL);
}
static MAKE_THISCALL(void, gbGfxManager_D3D_EndScene_wrapper, struct gbGfxManager_D3D *this)
{
    call_preendscene_hooks();
    gbGfxManager_D3D_EndScene(this);
    call_postpresent_hooks();
}
static void init_preendscene_postpresent_hook()
{
    INIT_WRAPPER_VFPTR(gbGfxManager_D3D_EndScene_wrapper, gboffset + 0x100D66F0); 
}



// OnLostDevice and OnResetDevice hooks
void add_onlostdevice_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ONLOSTDEVICE, funcptr);
}
void call_onlostdevice_hooks()
{
    run_hooks(HOOKID_ONLOSTDEVICE, NULL);
}
void add_onresetdevice_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ONRESETDEVICE, funcptr);
}
void call_onresetdevice_hooks()
{
    run_hooks(HOOKID_ONRESETDEVICE, NULL);
}


// post d3d create hooks
void add_postd3dcreate_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_POSTD3DCREATE, funcptr);
}
static void PAL3_InitGFX_wrapper()
{
    PAL3_InitGFX();
    run_hooks(HOOKID_POSTD3DCREATE, NULL);
}
static void init_postd3dcreate_hook()
{
    INIT_WRAPPER_CALL(PAL3_InitGFX_wrapper, { 0x0040676C });
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
    run_hooks(HOOKID_POSTPAL3CREATE, NULL);
}
static MAKE_ASMPATCH(post_gamecreate)
{
    // this asmpatch shall run just before outputing 'Game Create OK ...' log message
    run_hooks(HOOKID_POSTGAMECREATE, NULL);
    PUSH_DWORD(0x005750A0);
}
static void PAL3_Destroy_wrapper()
{
    if (pal3_created) {
        run_hooks(HOOKID_PREPAL3DESTROY, NULL);
        pal3_created = 0;
    }
    PAL3_Destroy();
}
static void init_postpal3create_prepal3destroy_hook()
{
    INIT_ASMPATCH(post_gamecreate, 0x00406944, 5, "\x68\xA0\x50\x57\x00");
    INIT_WRAPPER_CALL(PAL3_Create_wrapper, { 0x0052BA30 });
    INIT_WRAPPER_CALL(PAL3_Destroy_wrapper, { 0x0052BAF7 });
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
        run_hooks_witharg(HOOKID_GAMEPAUSERESUME, &pause_state, NULL);
    }
}
static void PAL3_Update_wrapper(double deltaTime)
{
    set_pauseresume(0);
    PAL3_Update(deltaTime);
}
static void init_pauseresume_hook()
{
    INIT_WRAPPER_CALL(PAL3_Update_wrapper, { 0x0052BAE6 });
}



// atexit hook
static void *atexit_hooks_dyncode;
void call_atexit_hooks()
{
    game_hwnd = NULL; // game_hwnd is invalid at exit, set it to NULL
    
    run_hooks(HOOKID_ATEXIT, NULL);
}
static MAKE_ASMPATCH(atexit_normal)
{
    call_atexit_hooks();
    
    LINK_JMP(TOUINT(atexit_hooks_dyncode));
}
void add_atexit_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_ATEXIT, funcptr);
}
static void init_atexit_hook()
{
    INIT_ASMPATCH(atexit_normal, 0x0052BB04, 7, "\x5F\x5E\x5B\xC9\xC2\x10\x00");
    
    atexit_hooks_dyncode = alloc_dyncode_buffer(7);
    memcpy(atexit_hooks_dyncode, "\x5F\x5E\x5B\xC9\xC2\x10\x00", 7);
    flush_instruction_cache(atexit_hooks_dyncode, 7);
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
    run_hooks_witharg(HOOKID_GAMELOOP, &arg, NULL);
}
static MAKE_ASMPATCH(gameloop_normal)
{
    set_pauseresume(0);
    call_gameloop_hooks(GAMELOOP_NORMAL, NULL);
    LINK_JMP(0x0052BA5A);
}
static MAKE_ASMPATCH(gameloop_sleep)
{
    set_pauseresume(1);
    call_gameloop_hooks(GAMELOOP_SLEEP, NULL);
    Sleep(100);
}
static MAKE_THISCALL(int, gbBinkVideo_OpenFile, struct gbBinkVideo *this, const char *szFileName, HWND hWnd, int bChangeScreenMode, int nOpenFlag)
{
    int ret = gbBinkVideo_SFLB_OpenFile(this, szFileName, hWnd, bChangeScreenMode, nOpenFlag);
    call_gameloop_hooks(GAMEEVENT_MOVIE_ATOPEN, (void *) szFileName);
    return ret;
}

static MAKE_THISCALL(int, gbBinkVideo_DoModal_wrapper, struct gbBinkVideo *this, int bCanSkip)
{
    int ret;
    if (this->m_hBink) call_gameloop_hooks(GAMEEVENT_MOVIE_ATBEGIN, NULL);
    ret = gbBinkVideo_DoModal(this, bCanSkip);
    call_gameloop_hooks(GAMEEVENT_MOVIE_ATEND, NULL);
    return ret;
}
static void gameloop_movie()
{
    set_pauseresume(0);
    call_gameloop_hooks(GAMELOOP_MOVIE, NULL);
}
static MAKE_ASMPATCH(gameloop_movie_part1)
{
    if (R_EAX != 0) {
        gameloop_movie();
        LINK_JMP(0x0052540C);
    } else {
        R_ECX = M_DWORD(R_EBP - 0x4);
    }
}
static MAKE_ASMPATCH(gameloop_movie_part2)
{
    if (R_EAX != 0) {
        LINK_JMP(0x00525473);
    } else {
        gameloop_movie();
        LINK_JMP(0x0052540C);
    }
}



static void init_gameloop_hook()
{    
    // patch main game loop
    INIT_ASMPATCH(gameloop_normal, 0x0052BAF2, 5, "\xE9\x63\xFF\xFF\xFF");
    INIT_ASMPATCH(gameloop_sleep, 0x0052BAD1, 8, "\x6A\x64\xFF\x15\x40\x80\x55\x00");

    // patch movie loop
    make_jmp(0x00525279, gbBinkVideo_OpenFile);
    INIT_WRAPPER_CALL(gbBinkVideo_DoModal_wrapper, {
        0x00406C07,
        0x0043FF8B,
        0x0045E047,
        0x004B2C84,
    });
    INIT_ASMPATCH(gameloop_movie_part1, 0x00525456, 7, "\x85\xC0\x75\xB2\x8B\x4D\xFC");
    INIT_ASMPATCH(gameloop_movie_part2, 0x00525462, 6, "\x85\xC0\x75\x0D\xEB\xA4");
}



// GetCursorPos hook
static BOOL WINAPI GetCursorPos_wrapper(LPPOINT lpPoint)
{
    BOOL ret = GetCursorPos(lpPoint);
    if (ret) run_hooks_witharg(HOOKID_GETCURSORPOS, lpPoint, NULL);
    return ret;
}
void add_getcursorpos_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_GETCURSORPOS, funcptr);
}
static void init_getcursorpos_hook()
{
    INIT_WRAPPER_CALL(GetCursorPos_wrapper, {
        0x00402A0C,
        0x00402B07,
        0x00402C72,
        0x004CA069,
    });
    // NOTE: No need to hook GetCursorPos() for IDirect3DDevice9::SetCursorPosition()
    //       see documentation for details.
}




// SetCursorPos hook
static BOOL WINAPI SetCursorPos_wrapper(int X, int Y)
{
    POINT pt = (POINT) { .x = X, .y = Y };
    run_hooks_reverse_witharg(HOOKID_SETCURSORPOS, &pt, NULL);
    return SetCursorPos(pt.x, pt.y);
}
void add_setcursorpos_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_SETCURSORPOS, funcptr);
}
static void init_setcursorpos_hook()
{
    make_jmp(0x00402ADA, SetCursorPos_wrapper);
}





// WndProc hooks

static int wndproc_brkcond(void *arg)
{
    struct wndproc_hook_data *data = arg;
    return data->processed;
}
static int call_wndproc_hook(int hookid, HWND *hWnd, UINT *Msg, WPARAM *wParam, LPARAM *lParam, LRESULT *retvalue)
{
    struct wndproc_hook_data data = {
        .hWnd = *hWnd,
        .Msg = *Msg,
        .wParam = *wParam,
        .lParam = *lParam,
        .retvalue = *retvalue,
        .processed = 0,
    };
    
    run_hooks_witharg(hookid, &data, wndproc_brkcond);
    
    if (data.processed) {
        *hWnd = data.hWnd;
        *Msg = data.Msg;
        *wParam = data.wParam;
        *lParam = data.lParam;
        *retvalue = data.retvalue;
        return 1;
    } else {
        return 0;
    }
}

void add_prewndproc_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_PREWNDPROC, funcptr);
}
int call_prewndproc_hook(HWND *hWnd, UINT *Msg, WPARAM *wParam, LPARAM *lParam, LRESULT *retvalue)
{
    return call_wndproc_hook(HOOKID_PREWNDPROC, hWnd, Msg, wParam, lParam, retvalue);
}
void add_postwndproc_hook(void (*funcptr)(void *))
{
    add_hook(HOOKID_POSTWNDPROC, funcptr);
}
int call_postwndproc_hook(HWND *hWnd, UINT *Msg, WPARAM *wParam, LPARAM *lParam, LRESULT *retvalue)
{
    return call_wndproc_hook(HOOKID_POSTWNDPROC, hWnd, Msg, wParam, lParam, retvalue);
}


// GRPinput keyboard state hook
void add_grpkbdstate_hook(void (*funcptr)(void))
{
    add_hook(HOOKID_GRPKBDSTATE, funcptr);
}
static MAKE_ASMPATCH(post_kbdstate_update)
{
    // state in g_input.m_keyRaw
    run_hooks(HOOKID_GRPKBDSTATE, NULL);
    R_EBX = R_ESI + 0x1030;
}
static void init_grpkbdstate_hook()
{
    INIT_ASMPATCH(post_kbdstate_update, 0x00402C1A, 6, "\x8D\x9E\x30\x10\x00\x00");
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
    init_grpkbdstate_hook();
}
