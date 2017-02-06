#ifndef PAL3PATCH_PAL3PATCH_H
#define PAL3PATCH_PAL3PATCH_H

#define EXTERNAL_UNPACKER "PAL3unpack.dll"
#define EXTERNAL_UNPACKER_FIXED "PAL3unpack_fixed.dll"
extern const char *unpacker_module_name;
extern unsigned gboffset;
#define PAL3_KERNEL32_IATBASE TOPTR(0x0056A024)
#define PAL3_WINMM_IATBASE TOPTR(0x0056A1C0)
#define PAL3_USER32_IATBASE TOPTR(0x0056A134)


#endif
