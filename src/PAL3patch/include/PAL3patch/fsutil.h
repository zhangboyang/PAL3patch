#ifndef PAL3PATCH_FSUTIL_H
#define PAL3PATCH_FSUTIL_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int utf8_filepath_to_wstr_fullpath(const char *filepath, wchar_t *buf, size_t bufsize, wchar_t **filepart);
extern PATCHAPI int replace_filepart_with_relpath(const wchar_t *filepath, const wchar_t *relpath, wchar_t *buf, size_t bufsize, wchar_t **filepart);
extern PATCHAPI char *get_filepart(const char *filepath);
extern PATCHAPI wchar_t *get_wfilepart(const wchar_t *wfilepath);
extern PATCHAPI int is_relpath(const char *filepath);
extern PATCHAPI char *read_file_as_cstring(const char *filepath);
extern PATCHAPI int enum_files(const char *dirpath, const char *pattern, void (*func)(const char *filepath, void *arg), void *arg);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#endif
#endif
