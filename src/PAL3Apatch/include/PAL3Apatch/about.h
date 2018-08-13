#ifndef PAL3APATCH_ABOUT_H
#define PAL3APATCH_ABOUT_H
// PATCHAPI DEFINITIONS

#if defined(__GNUC__) && defined(__VERSION__)
#define BUILD_COMPILER "gcc " __VERSION__
#elif defined(_MSC_VER)
#define BUILD_COMPILER "Microsoft C " TOSTR(_MSC_VER)
#elif defined(__TINYC__)
#define BUILD_COMPILER "Tiny C Compiler " TOSTR(__TINYC__)
#else
#define BUILD_COMPILER "unknown C compiler"
#endif

#define BUILD_DATE __DATE__ ", " __TIME__

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
