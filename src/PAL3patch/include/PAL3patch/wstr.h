#ifndef PAL3PATCH_WSTR_H
#define PAL3PATCH_WSTR_H
// PATCHAPI DEFINITIONS

extern PATCHAPI wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp);
extern PATCHAPI char *wcs2cs_alloc(const wchar_t *wstr, UINT dst_cp);
extern PATCHAPI char *cs2cs_alloc(const char *cstr, UINT src_cp, UINT dst_cp);

extern PATCHAPI wchar_t *cs2wcs_managed(const char *cstr, UINT src_cp, wchar_t **pptr);
extern PATCHAPI char *wcs2cs_managed(const wchar_t *wstr, UINT dst_cp, char **pptr);
extern PATCHAPI char *cs2cs_managed(const char *cstr, UINT src_cp, UINT dst_cp, char **pptr);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

extern wchar_t *cs2wcs(const char *cstr, UINT src_cp);
extern char *wcs2cs(const wchar_t *wstr, UINT dst_cp);
extern char *cs2cs(const char *cstr, UINT src_cp, UINT dst_cp);

#endif
#endif
