#ifndef PAL3PATCHCONFIG_MISC
#define PAL3PATCHCONFIG_MISC

extern int is_win9x();
extern int is_winxp_or_later();

extern HANDLE acquire_mutex(LPCSTR lpName, DWORD dwMilliseconds);
extern void release_mutex(HANDLE hMutex);

extern LPOSVERSIONINFO GetVersionEx_cached();

extern void DoEvents();
extern void *Malloc(size_t n);

extern __declspec(noreturn) void die(int status);

#endif
