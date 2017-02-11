#ifndef PAL3PATCH_HOOK_H
#define PAL3PATCH_HOOK_H


// hook framework
#define MAX_HOOKS 50
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
    HOOKID_PREPAL3DESTROY,
    HOOKID_GAMEPAUSERESUME,
    
    MAX_HOOK_TYPES // EOF
};

// pre-EndScene and post-Present hooks
extern void add_preendscene_hook(void (*funcptr)(void));
extern void add_postpresent_hook(void (*funcptr)(void));
extern void call_preendscene_hooks(void);
extern void call_postpresent_hooks(void);


// OnLostDevice and OnResetDevice hooks
extern void add_onlostdevice_hook(void (*funcptr)(void));
extern void call_onlostdevice_hooks(void);
extern void add_onresetdevice_hook(void (*funcptr)(void));
extern void call_onresetdevice_hooks(void);

// post d3d create hooks
extern void add_postd3dcreate_hook(void (*funcptr)(void));

// post PAL3::Create and pre PAL3::Destroy hooks
extern void add_postpal3create_hook(void (*funcptr)(void));
extern void add_prepal3destroy_hook(void (*funcptr)(void));

// game pause and resume hooks
// data is pointer to int, zero => running, non-zero => paused
extern void add_pauseresume_hook(void (*funcptr)(void *));
extern void set_pauseresume(int state);

// atexit hooks
extern void add_atexit_hook(void (*funcptr)(void));

// gameloop hooks
extern void add_gameloop_hook(void (*funcptr)(void *));
extern void call_gameloop_hooks(int type, void *data);

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
extern void add_getcursorpos_hook(void (*funcptr)(void *));

// SetCursorPos hook, data is pointer to POINT
extern void add_setcursorpos_hook(void (*funcptr)(void *));

// init all hooks
extern void init_hooks(void);


#endif
