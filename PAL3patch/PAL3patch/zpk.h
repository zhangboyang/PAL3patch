#ifndef PAL3PATCH_ZPK_H
#define PAL3PATCH_ZPK_H

extern void load_zpk(const char *zpkfn);
extern int zpk_getfile(const void *sha1buf, void **dataptr, unsigned *datalen);
extern void unload_zpk();

#endif
