#ifndef PAL3APATCH_WAL_H
#define PAL3APATCH_WAL_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int wal_replace(char *dst[], char *src[], int n, const char *sum);
extern PATCHAPI int wal_check(char *dst[], char *src[], int n, const char *sum);

extern PATCHAPI int wal_replace1(const char *dst, const char *src, const char *sum);
extern PATCHAPI int wal_check1(const char *dst, const char *src, const char *sum);

#endif
