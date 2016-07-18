#ifndef PAL3PATCH_COMMON_H
#define PAL3PATCH_COMMON_H

// common constants
#define MAXLINE 4096

#define PAL3_OEP 0x55507C


// framework.c
extern void memcpy_to_process(unsigned dest, void *src, unsigned size);
extern void memcpy_from_process(void *dest, unsigned src, unsigned size);
extern void make_jmp(unsigned addr, void *jtarget);

// misc.c
#define fail(fmt, ...) __fail(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __fail(const char *file, int line, const char *func, const char *fmt, ...);




#endif
