#ifndef PAL3APATCH_PAL3APATCH_H
#define PAL3APATCH_PAL3APATCH_H
// PATCHAPI DEFINITIONS

extern PATCHAPI unsigned gboffset;
extern PATCHAPI HINSTANCE patch_hinstDLL;

extern PATCHAPI void launcher_entry(unsigned oep_addr);
extern PATCHAPI void sforce_unpacker_entry(void);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define EXTERNAL_UNPACKER "PAL3Aunpack.dll"
#define EXTERNAL_UNPACKER_FIXED "PAL3Aunpack_fixed.dll"

#define PAL3A_KERNEL32_IATBASE TOPTR(0x00558024)
#define PAL3A_WINMM_IATBASE TOPTR(0x005581C8)
#define PAL3A_USER32_IATBASE TOPTR(0x00558148)

extern unsigned sforce_unpacker_init(void);

#endif
#endif
