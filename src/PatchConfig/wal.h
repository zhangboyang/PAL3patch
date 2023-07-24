#ifndef PAL3PATCHCONFIG_WAL
#define PAL3PATCHCONFIG_WAL

extern int wal_replace(char *dst[], char *src[], int n, const char *sum);
extern int wal_check(char *dst[], char *src[], int n, const char *sum);

extern int wal_replace1(const char *dst, const char *src, const char *sum);
extern int wal_check1(const char *dst, const char *src, const char *sum);

#endif
