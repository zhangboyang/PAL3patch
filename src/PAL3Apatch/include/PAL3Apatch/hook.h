#ifndef PAL3APATCH_HOOK_H
#define PAL3APATCH_HOOK_H
// PATCHAPI DEFINITIONS


// pre-EndScene and post-Present hooks
extern PATCHAPI void add_preendscene_hook(void (*funcptr)(void));
extern PATCHAPI void add_postpresent_hook(void (*funcptr)(void));
extern PATCHAPI void call_preendscene_hooks(void);
extern PATCHAPI void call_postpresent_hooks(void);


// OnLostDevice and OnResetDevice hooks
extern PATCHAPI void add_onlostdevice_hook(void (*funcptr)(void));
extern PATCHAPI void call_onlostdevice_hooks(void);
extern PATCHAPI void add_onresetdevice_hook(void (*funcptr)(void));
extern PATCHAPI void call_onresetdevice_hooks(void);

// post d3d create hooks
extern PATCHAPI void add_postd3dcreate_hook(void (*funcptr)(void));

// post PAL3::Create and pre PAL3::Destroy hooks
extern PATCHAPI void add_postpal3create_hook(void (*funcptr)(void));
extern PATCHAPI void add_postgamecreate_hook(void (*funcptr)(void));
extern PATCHAPI void add_prepal3destroy_hook(void (*funcptr)(void));

// game pause and resume hooks
// data is pointer to int, zero => running, non-zero => paused
extern PATCHAPI void add_pauseresume_hook(void (*funcptr)(void *));
extern PATCHAPI void set_pauseresume(int state);

// atexit hooks
extern PATCHAPI void call_atexit_hooks(void);
extern PATCHAPI void add_atexit_hook(void (*funcptr)(void));

// gameloop hooks
extern PATCHAPI void add_gameloop_hook(void (*funcptr)(void *));
extern PATCHAPI void call_gameloop_hooks(int type, void *data);

enum game_loop_type {
    // loop
    GAMELOOP_NORMAL,
    GAMELOOP_SLEEP,
    GAMELOOP_DEVICELOST,
    GAMELOOP_MOVIE,
    
    // event
    GAMEEVENT_MOVIE_ATOPEN, // data is pointer to filename string, const char *
    GAMEEVENT_MOVIE_ATBEGIN,
    GAMEEVENT_MOVIE_ATEND,
};
struct game_loop_hook_data {
    enum game_loop_type type;
    void *data;
};


// GetCursorPos hook, data is pointer to POINT
extern PATCHAPI void add_getcursorpos_hook(void (*funcptr)(void *));

// SetCursorPos hook, data is pointer to POINT
extern PATCHAPI void add_setcursorpos_hook(void (*funcptr)(void *));


// WndProc hook
struct wndproc_hook_data {
    HWND hWnd;
    UINT Msg;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT retvalue;
    int processed;
};

extern PATCHAPI void add_prewndproc_hook(void (*funcptr)(void *));
extern PATCHAPI void add_postwndproc_hook(void (*funcptr)(void *));


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

// hook framework
#define MAX_HOOKS 100
enum hook_type {
    HOOKID_ATEXIT,
    HOOKID_GAMELOOP,
    HOOKID_GETCURSORPOS,
    HOOKID_SETCURSORPOS,
    HOOKID_POSTD3DCREATE,
    HOOKID_ONLOSTDEVICE,
    HOOKID_ONRESETDEVICE,
    HOOKID_PREENDSCENE,
    HOOKID_POSTPRESENT,
    HOOKID_POSTPAL3CREATE,
    HOOKID_POSTGAMECREATE,
    HOOKID_PREPAL3DESTROY,
    HOOKID_GAMEPAUSERESUME,
    HOOKID_PREWNDPROC,
    HOOKID_POSTWNDPROC,
    
    MAX_HOOK_TYPES // EOF
};

// internal uses only
extern int call_prewndproc_hook(HWND *hWnd, UINT *Msg, WPARAM *wParam, LPARAM *lParam, LRESULT *retvalue);
extern int call_postwndproc_hook(HWND *hWnd, UINT *Msg, WPARAM *wParam, LPARAM *lParam, LRESULT *retvalue);

// init all hooks
extern void init_hooks(void);

#endif
#endif
