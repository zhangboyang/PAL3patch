#ifndef PAL3PATCH_HOOK_H
#define PAL3PATCH_HOOK_H


// hook framework
#define MAX_HOOKS 20
enum hook_type {
    HOOKID_ATEXIT,
    HOOKID_GAMELOOP,
    HOOKID_GETCURSORPOS,
    HOOKID_SETCURSORPOS,
    HOOKID_POSTD3DCREATE,
    HOOKID_ONLOSTDEVICE,
    HOOKID_ONRESETDEVICE,
    HOOKID_PREENDSCENE,
    MAX_HOOK_TYPES // EOF
};

// EndScene hooks
extern void add_preendscene_hook(void (*funcptr)(void));
extern void call_preendscene_hooks();
extern void init_preendscene_hook();

// OnLostDevice and OnResetDevice hooks
extern void add_onlostdevice_hook(void (*funcptr)(void));
extern void call_onlostdevice_hooks();
extern void add_onresetdevice_hook(void (*funcptr)(void));
extern void call_onresetdevice_hooks();

// post d3d create hooks
extern void add_postd3dcreate_hook(void (*funcptr)(void));
extern void init_postd3dcreate_hook();


// atexit hooks
extern void add_atexit_hook(void (*funcptr)(void));
extern void init_atexit_hook();

// gameloop hooks
extern int gameloop_hookflag;
extern void add_gameloop_hook(void (*funcptr)(void));
extern void call_gameloop_hooks(int flag);
extern void init_gameloop_hook();

enum game_loop_type {
    GAMELOOP_NORMAL,
    GAMELOOP_SLEEP,
    GAMELOOP_MOVIE,
    GAMELOOP_MOVIE_ATEXIT,
    GAMELOOP_DEVICELOST,
};


// GetCursorPos hook
extern LPPOINT getcursorpos_hook_lppoint;
extern BOOL getcursorpos_hook_ret;
extern void add_getcursorpos_hook(void (*funcptr)(void));
extern void init_getcursorpos_hook();

// SetCursorPos hook
extern POINT setcursorpos_hook_point;
extern void add_setcursorpos_hook(void (*funcptr)(void));
extern void init_setcursorpos_hook();

// init all hooks
extern void init_hooks();


#endif
