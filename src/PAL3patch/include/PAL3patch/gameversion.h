#ifndef PAL3PATCH_GAMEVERSION_H
#define PAL3PATCH_GAMEVERSION_H
// PATCHAPI DEFINITIONS

extern PATCHAPI void disable_gameversion_check(void);

#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

extern void check_gameversion(void);

#endif
#endif