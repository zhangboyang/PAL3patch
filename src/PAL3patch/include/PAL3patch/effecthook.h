#ifndef PAL3PATCH_EFFECTHOOK_H
#define PAL3PATCH_EFFECTHOOK_H
// PATCHAPI DEFINITIONS


extern PATCHAPI void add_effect_hook(const char *eff_file, const char *old_str, const char *new_str);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define MAX_EFFECTHOOKS 100
extern PATCHAPI void init_effect_hooks(void);

#endif
#endif
