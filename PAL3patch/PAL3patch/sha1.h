#ifndef PAL3PATCH_SHA1_H
#define PAL3PATCH_SHA1_H

#define SHA1_BYTE 20
#define SHA1_STR_SIZE (SHA1_BYTE * 2 + 1)

extern void sha1_hash_buffer(const void *databuf, int datalen, unsigned char *hashbuf);
extern char *sha1_tostr(const void *sha1buf);
extern void sha1_fromstr(void *sha1buf, const char *sha1str);

#endif
