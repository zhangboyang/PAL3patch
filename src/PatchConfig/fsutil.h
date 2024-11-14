#ifndef PAL3PATCHCONFIG_FSUTIL
#define PAL3PATCHCONFIG_FSUTIL

extern int create_dir(const char *dirpath);
extern int dir_exists(const char *dirpath);
extern int file_exists(const char *filepath);
extern int reset_attrib(const char *filepath);
extern FILE *robust_fopen(const char *filename, const char *mode);
extern int safe_fclose(FILE **fp);
extern int robust_unlink(const char *filename);

#endif
