#include "common.h"

#define SAFE_CS2WCS L"cs2wcs() failed."
#define SAFE_WCS2CS "wcs2cs() failed."
#define NO_GLOBAL_MANAGED

// convert a mbcs-string to an unicode-string with given codepage
// will alloc memory, don't forget to free()
wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp)
{
    wchar_t *ret = NULL;
    size_t len;
    
    // get string length first
    len = MultiByteToWideChar(src_cp, 0, cstr, -1, NULL, 0);
    if (len == 0) goto fail;
    
    // alloc buffer
    ret = (wchar_t *) malloc(sizeof(wchar_t) * len);
    if (!ret) goto fail;
    
    // do real convert
    if (MultiByteToWideChar(src_cp, 0, cstr, -1, ret, len) == 0) goto fail;
    return ret;

fail:
    free(ret);
#ifdef SAFE_CS2WCS
    return wcsdup(SAFE_CS2WCS);
#else
    return NULL;
#endif
}

// convert an unicode-string to a mbcs-string with given codepage
// will alloc memory, don't forget to free()
char *wcs2cs_alloc(const wchar_t *wstr, UINT dst_cp)
{
    char *ret = NULL;
    size_t len;
    
    // get string length first
    len = WideCharToMultiByte(dst_cp, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len == 0) goto fail;
    
    // alloc buffer
    ret = (char *) malloc(len);
    if (!ret) goto fail;
    
    // do real convert
    if (WideCharToMultiByte(dst_cp, 0, wstr, -1, ret, len, NULL, NULL) == 0) goto fail;
    return ret;
    
fail:
    free(ret);
#ifdef SAFE_WCS2CS
    return strdup(SAFE_WCS2CS);
#else
    return NULL;
#endif
}


// convert a mbcs-string in src_cp to a mbcs-string in dst_cp
// will alloc memory, don't forget to free()
char *cs2cs_alloc(const char *cstr, UINT src_cp, UINT dst_cp)
{
    wchar_t *wstr;
    char *ret = NULL;
    
    // convert to unicode-string first
    wstr = cs2wcs_alloc(cstr, src_cp);
    if (!wstr) goto done;
    
    // convert back to mbcs-string
    ret = wcs2cs_alloc(wstr, dst_cp);
    
done:
    free(wstr);
    return ret;
}



// return pointer to managed memory
wchar_t *cs2wcs_managed(const char *cstr, UINT src_cp, wchar_t **pptr)
{
    free(*pptr);
    return *pptr = cs2wcs_alloc(cstr, src_cp);
}
char *wcs2cs_managed(const wchar_t *wstr, UINT dst_cp, char **pptr)
{
    free(*pptr);
    return *pptr = wcs2cs_alloc(wstr, dst_cp);
}
char *cs2cs_managed(const char *cstr, UINT src_cp, UINT dst_cp, char **pptr)
{
    free(*pptr);
    return *pptr = cs2cs_alloc(cstr, src_cp, dst_cp);
}


#ifndef NO_GLOBAL_MANAGED

// return pointer to globally managed memory (vaild until next call)
wchar_t *cs2wcs(const char *cstr, UINT src_cp)
{
    static wchar_t *ptr = NULL;
    return cs2wcs_managed(cstr, src_cp, &ptr);
}
char *wcs2cs(const wchar_t *wstr, UINT dst_cp)
{
    static char *ptr = NULL;
    return wcs2cs_managed(wstr, dst_cp, &ptr);
}
char *cs2cs(const char *cstr, UINT src_cp, UINT dst_cp)
{
    static char *ptr = NULL;
    return cs2cs_managed(cstr, src_cp, dst_cp, &ptr);
}

#endif
