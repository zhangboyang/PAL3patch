#ifndef PAL3PATCHCONFIG_UACVIRTUALIZATION
#define PAL3PATCHCONFIG_UACVIRTUALIZATION

extern int TrySetUACVirtualization(bool en);
extern void GetUACVirtualizedCurrentDirectory(LPTSTR out, DWORD outsz);

#endif
