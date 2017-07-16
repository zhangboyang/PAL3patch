#include "stdafx.h"

// convert a mbcs-string to unicode-string with given codepage
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
    return L"cs2wcs() failed.";
}

// convert a unicode-string to mbcs-string with given codepage
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
    return "wcs2cs() failed.";
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

// return pointer to global memory (vaild until next call)
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
