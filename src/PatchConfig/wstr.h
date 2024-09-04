#ifndef PAL3PATCHCONFIG_WSTR
#define PAL3PATCHCONFIG_WSTR

extern char *utf16_to_utf8(const wchar_t *s);
extern wchar_t *utf8_to_utf16(const char *s);

extern wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp);
extern char *wcs2cs_alloc(const wchar_t *wstr, UINT dst_cp);
extern char *cs2cs_alloc(const char *cstr, UINT src_cp, UINT dst_cp);

extern wchar_t *cs2wcs_managed(const char *cstr, UINT src_cp, wchar_t **pptr);
extern char *wcs2cs_managed(const wchar_t *wstr, UINT dst_cp, char **pptr);
extern char *cs2cs_managed(const char *cstr, UINT src_cp, UINT dst_cp, char **pptr);

extern wchar_t *cs2wcs(const char *cstr, UINT src_cp);
extern char *wcs2cs(const wchar_t *wstr, UINT dst_cp);
extern char *cs2cs(const char *cstr, UINT src_cp, UINT dst_cp);

#endif
