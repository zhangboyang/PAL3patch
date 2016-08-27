#ifndef PAL3PATCHCONFIG_HASHSHA1
#define PAL3PATCHCONFIG_HASHSHA1

#define SHA1_BYTE 20
#define SHA1_STR_SIZE (SHA1_BYTE * 2 + 1)

extern int GetFileSHA1(const char *fn, char *buf);

#endif