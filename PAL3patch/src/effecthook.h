#ifndef PAL3PATCH_EFFECTHOOK_H
#define PAL3PATCH_EFFECTHOOK_H

#define MAX_EFFECTHOOKS 50

extern void add_effect_hook(const char *eff_file, const char *old_str, const char *new_str);
extern void init_effect_hooks(void);

#endif
