#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

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

static char *get_filepart(const char *filepath)
{
    char *t;
    char *filepart = (char *) filepath;
    
    t = strrchr(filepart, '\\');
    if (t) filepart = t + 1;
    t = strrchr(filepart, '/');
    if (t) filepart = t + 1;
    
    return filepart;
}

static char *utf16_to_utf8(const wchar_t *s)
{
    char *buf = malloc(wcslen(s) * 3 + 1);
    char *p = buf;
    while (*s) {
        unsigned c = 0xfffd;
        unsigned short w1 = *s++;
        if ((w1 & 0xf800) != 0xd800) {
            c = w1;
        } else if ((w1 & 0xfc00) == 0xd800) {
            unsigned short w2 = *s;
            if ((w2 & 0xfc00) == 0xdc00) {
                c = (((w1 & 0x3ff) << 10) | (w2 & 0x3ff)) + 0x10000;
                s++;
            }
        }
        if (c < (1 << 7)) {
            *p++ = c;
        } else if (c < (1 << 11)) {
            *p++ = 0xc0 | (c >> 6);
            *p++ = 0x80 | (c & 0x3f);
        } else if (c < (1 << 16)) {
            *p++ = 0xe0 | (c >> 12);
            *p++ = 0x80 | ((c >> 6) & 0x3f);
            *p++ = 0x80 | (c & 0x3f);
        } else {
            *p++ = 0xf0 | (c >> 18);
            *p++ = 0x80 | ((c >> 12) & 0x3f);
            *p++ = 0x80 | ((c >> 6) & 0x3f);
            *p++ = 0x80 | (c & 0x3f);
        }
    }
    *p = 0;
    return buf;
}

static wchar_t *utf8_to_utf16(const char *s)
{
    wchar_t *buf = malloc(sizeof(wchar_t) * (strlen(s) + 1));
    wchar_t *p = buf;
    while (*s) {
        int i, n;
        unsigned c;
        unsigned char b = *s++;
        if (b <= 0x7f) {
            n = 0; c = b;
        } else if (b < 0xc2) {
            n = 0; c = 0xfffd;
        } else if (b <= 0xdf) {
            n = 1; c = b & 0x1f;
        } else if (b <= 0xef) {
            n = 2; c = b & 0xf;
        } else if (b <= 0xf4) {
            n = 3; c = b & 0x7;
        } else {
            n = 0; c = 0xfffd;
        }
        for (i = 1; i <= n; i++) {
            int l = 0x80, h = 0xbf;
            if (i == 1) {
                switch (b) {
                case 0xe0: l = 0xa0; break;
                case 0xed: h = 0x9f; break;
                case 0xf0: l = 0x90; break;
                case 0xf4: h = 0x8f; break;
                }
            }
            b = *s;
            if (b < l || b > h) {
                c = 0xfffd; break;
            }
            c = (c << 6) | (b & 0x3f);
            s++;
        }
        if (c < 0x10000) {
            *p++ = c;
        } else {
            c -= 0x10000;
            *p++ = 0xd800 | (c >> 10);
            *p++ = 0xdc00 | (c & 0x3ff);
        }
    }
    *p = 0;
    return buf;
}

// convert a mbcs-string to an unicode-string with given codepage
// will alloc memory, don't forget to free()
static wchar_t *cs2wcs_alloc(const char *cstr, UINT src_cp)
{
    if (src_cp == CP_UTF8) {
        return utf8_to_utf16(cstr);
    }
    
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
    if (dst_cp == CP_UTF8) {
        return utf16_to_utf8(wstr);
    }
    
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
    char *u8path = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwSize, dwRead;
    UINT cp;
    
    // check command args
    if (argc < 2) goto fail;
    
    // set stdout to binary
    setmode(fileno(stdout), O_BINARY);
    
    // convert file-path
    u8path = cs2cs_alloc(argv[1], CP_ACP, CP_UTF8);
    if (!u8path) goto fail;
    
    // open input file
    hFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) goto fail;
    
    // get input file size
    dwSize = GetFileSize(hFile, NULL);
    if (dwSize == INVALID_FILE_SIZE) goto fail;
    
    // allocate memory and read file
    data = malloc(dwSize + 1);
    if (!ReadFile(hFile, data, dwSize, &dwRead, NULL) || dwSize != dwRead) goto fail;
    data[dwSize] = 0;
    
    // detect original codepage
    cp = CP_ACP;
    if (str_remove_utf8_bom(data)) {
        cp = CP_UTF8;
    }
    
    // do convert
    newdata = cs2cs_alloc(data, cp, CP_UTF8);
    if (!newdata) goto fail;
    
    // write output to stdout
    fprintf(stdout, "#define TCCPLUGIN_FILE \"%s\"\n", get_filepart(u8path));
    fputs(newdata, stdout);
    
    ret = 0;

done:
    free(data);
    free(newdata);
    free(u8path);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return ret;
fail:
    puts("#error \"can't convert to UTF-8.\"");
    goto done;
}
