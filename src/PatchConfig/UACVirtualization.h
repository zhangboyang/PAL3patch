#ifndef PAL3PATCHCONFIG_UACVIRTUALIZATION
#define PAL3PATCHCONFIG_UACVIRTUALIZATION

extern int TrySetUACVirtualization(BOOL enabled);
extern void GetUACVirtualizedCurrentDirectory(LPTSTR out, DWORD outsz);

#endif

