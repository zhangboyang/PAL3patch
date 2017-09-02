#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#define MAXSIZE (1024 * 1024 * 64)
#define MAXLINE 1

#define UTF8_BOM_STR "\xEF\xBB\xBF"
#define UTF8_BOM_LEN 3

static int str_remove_utf8_bom(char *str)
{
    int ret = 0;
    if (strncmp(str, UTF8_BOM_STR, UTF8_BOM_LEN) == 0) {
        memmove(str, str + UTF8_BOM_LEN, strlen(str + UTF8_BOM_LEN) + 1);
        ret = 1;
    }
    return ret;
}


// convert a mbcs-string to an unicode-string with given codepage
// will alloc memory, don't forget to free()
static wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp)
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
static char *wcs2cs_alloc(const wchar_t *wstr, UINT dst_cp)
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
static char *cs2cs_alloc(const char *cstr, UINT src_cp, UINT dst_cp)
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


int main(int argc, char *argv[])
{
    int ret = 1;
    char *data = NULL;
    char *newdata = NULL;
    FILE *fp = NULL;
    int len;
    int cur;
    UINT cp;
    
    // check command args
    if (argc < 2) goto fail;
    
    // set stdout to binary
    setmode(fileno(stdout), O_BINARY);
    
    // alloc memory
    data = malloc(MAXSIZE);

    // read input file
    fp = fopen(argv[1], "rb");
    if (!fp) goto fail;
    for (len = 0; (cur = fread(data + len, 1, (MAXSIZE - len < MAXLINE ? MAXSIZE - len : MAXLINE), fp)); len += cur);
    fclose(fp); fp = NULL;
    data[len < MAXSIZE ? len : MAXSIZE - 1] = 0;
    
    // detect original codepage
    cp = CP_ACP;
    if (str_remove_utf8_bom(data)) {
        cp = CP_UTF8;
    }
    
    // do convert
    newdata = cs2cs_alloc(data, cp, CP_UTF8);
    if (!newdata) goto fail;
    
    // write output to stdout
    fputs(newdata, stdout);
    
    ret = 0;

done:
    free(data);
    free(newdata);
    if (fp) fclose(fp);
    return ret;
fail:
    puts("#error \"can't convert to UTF-8.\"");
    goto done;
}
