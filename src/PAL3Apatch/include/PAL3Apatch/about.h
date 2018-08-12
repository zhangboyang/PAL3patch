#ifndef PAL3APATCH_ABOUT_H
#define PAL3APATCH_ABOUT_H
// PATCHAPI DEFINITIONS

extern PATCHAPI const char patch_version[];
extern PATCHAPI const char patch_build_date[];
extern PATCHAPI const char patch_build_compiler[];
extern PATCHAPI const char patch_build_info[];

#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define build_date       patch_build_date
#define build_compiler   patch_build_compiler
#define build_info       patch_build_info

extern void show_about(void);

#endif
#endif
