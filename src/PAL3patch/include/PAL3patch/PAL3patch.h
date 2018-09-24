#ifndef PAL3PATCH_PAL3PATCH_H
#define PAL3PATCH_PAL3PATCH_H
// PATCHAPI DEFINITIONS

extern PATCHAPI unsigned gboffset;
extern PATCHAPI HINSTANCE patch_hinstDLL;

extern PATCHAPI void launcher_entry(unsigned oep_addr);
extern PATCHAPI void sforce_unpacker_entry(void);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define hinstDLL patch_hinstDLL

#define EXTERNAL_UNPACKER "PAL3unpack.dll"
#define EXTERNAL_UNPACKER_FIXED "PAL3unpack_fixed.dll"

#define PAL3_KERNEL32_IATBASE TOPTR(0x0056A024)
#define PAL3_WINMM_IATBASE TOPTR(0x0056A1C0)
#define PAL3_USER32_IATBASE TOPTR(0x0056A134)

extern unsigned sforce_unpacker_init(void);

#endif
#endif
