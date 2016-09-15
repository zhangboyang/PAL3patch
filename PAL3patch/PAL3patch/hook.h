#ifndef PAL3PATCH_HOOK_H
#define PAL3PATCH_HOOK_H


// hook framework
#define MAX_HOOK_TYPES 5
#define MAX_HOOKS 10
enum hook_type {
    HOOKID_ATEXIT,
    HOOKID_GAMELOOP,
};

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
    GAMELOOP_DEVICELOST,
};

#endif
