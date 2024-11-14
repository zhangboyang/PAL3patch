#ifndef PAL3PATCH_FSUTIL_H
#define PAL3PATCH_FSUTIL_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int utf8_filepath_to_wstr_fullpath(const char *filepath, wchar_t *buf, size_t bufsize, wchar_t **filepart);
extern PATCHAPI int replace_filepart_with_relpath(const wchar_t *filepath, const wchar_t *relpath, wchar_t *buf, size_t bufsize, wchar_t **filepart);
extern PATCHAPI char *get_filepart(const char *filepath);
extern PATCHAPI wchar_t *get_wfilepart(const wchar_t *wfilepath);
extern PATCHAPI int is_relpath(const char *filepath);
extern PATCHAPI char *replace_extension(const char *filepath, const char *new_extension);
extern PATCHAPI char *read_file_as_cstring(const char *filepath);
extern PATCHAPI int enum_files(const char *dirpath, const char *pattern, void (*func)(const char *filepath, void *arg), void *arg);
extern PATCHAPI int create_dir(const char *dirpath);
extern PATCHAPI int dir_exists(const char *dirpath);
extern PATCHAPI int file_exists(const char *filepath);
extern PATCHAPI int reset_attrib(const char *filepath);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

// don't pass CRT objects (FILE *, errno, etc.) across DLL boundary
extern FILE *robust_fopen(const char *filename, const char *mode);
extern int safe_fclose(FILE **fp);
extern int robust_unlink(const char *filename);

#endif
#endif
