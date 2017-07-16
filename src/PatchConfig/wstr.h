#ifndef PAL3PATCHCONFIG_WSTR
#define PAL3PATCHCONFIG_WSTR

extern wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp);
extern char *wcs2cs_alloc(const wchar_t *wstr, UINT dst_cp);
extern wchar_t *cs2wcs_managed(const char *cstr, UINT src_cp, wchar_t **pptr);
extern char *wcs2cs_managed(const wchar_t *wstr, UINT dst_cp, char **pptr);
extern wchar_t *cs2wcs(const char *cstr, UINT src_cp);
extern char *wcs2cs(const wchar_t *wstr, UINT dst_cp);

#endif
