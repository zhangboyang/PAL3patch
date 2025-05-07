#ifndef PAL3PATCHCONFIG_UACVIRTUALIZATION
#define PAL3PATCHCONFIG_UACVIRTUALIZATION

extern int TrySetUACVirtualization(bool en);
extern void GetUACVirtualizedCurrentDirectory(LPCTSTR testfile, LPTSTR out, DWORD outsz);

#endif
