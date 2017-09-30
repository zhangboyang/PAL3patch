// misc functions
#include "common.h"


int is_win9x()
{
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
}

int str2int(const char *valstr)
{
    int result, ret;
    ret = sscanf(valstr, "%d", &result);
    if (ret != 1) fail("can't parse '%s' to integer.", valstr);
    return result;
}

double str2double(const char *valstr)
{
    int ret;
    double result;
    ret = sscanf(valstr, "%lf", &result);
    if (ret != 1) fail("can't parse '%s' to double.", valstr);
    return result;
}

int str_endswith(const char *a, const char *b)
{
    size_t lena = strlen(a), lenb = strlen(b);
    return lena >= lenb && strcmp(a + lena - lenb, b) == 0;
}
int str_iendswith(const char *a, const char *b)
{
    size_t lena = strlen(a), lenb = strlen(b);
    return lena >= lenb && stricmp(a + lena - lenb, b) == 0;
}

int str_remove_utf8_bom(char *str)
{
    int ret = 0;
    if (strncmp(str, UTF8_BOM_STR, UTF8_BOM_LEN) == 0) {
        memmove(str, str + UTF8_BOM_LEN, strlen(str + UTF8_BOM_LEN) + 1);
        ret = 1;
    }
    return ret;
}

char *str_rtrim(char *str, const char *charlist)
{
    char *end = str + strlen(str);
    while (end > str && strchr(charlist, end[-1])) end--;
    *end = 0;
    return str;
}
char *str_ltrim(char *str, const char *charlist)
{
    char *dst = str;
    char *src = str + strspn(str, charlist);
    if (dst != src) {
        while ((*dst++ = *src++));
    }
    return str;
}
char *str_trim(char *str, const char *charlist)
{
    return str_ltrim(str_rtrim(str, charlist), charlist);
}


char *strtok_r(char *str, const char *delim, char **saveptr)
{
    if (!(str = str ? str : *saveptr)) {
        return NULL;
    }
    if (!*(str += strspn(str, delim))) {
        return *saveptr = NULL;
    }
    if ((*saveptr = strpbrk(str, delim))) {
        *(*saveptr)++ = '\0';
    }
    return str;
}



int iabs(int x)
{
    return x >= 0 ? x : -x;
}

int imax(int a, int b)
{
    return a > b ? a : b;
}

int imin(int a, int b)
{
    return a < b ? a : b;
}

double fbound(double x, double low, double high)
{
    return fmin(fmax(x, low), high);
}

int fcmp(double a, double b)
{
    double d = a - b;
    if (fabs(d) <= eps) return 0;
    return d > 0 ? 1 : -1;
}

HMODULE GetModuleHandle_check(LPCSTR lpModuleName)
{
    HMODULE ret = LoadLibrary(lpModuleName);
    if (!ret) fail("can't find library '%s'.", lpModuleName);
    return ret;
}

HMODULE LoadLibrary_check(LPCSTR lpFileName)
{
    HMODULE ret = LoadLibrary(lpFileName);
    if (!ret) fail("can't load library '%s'.", lpFileName);
    return ret;
}

FARPROC GetProcAddress_check(HMODULE hModule, LPCSTR lpProcName)
{
    FARPROC ret = GetProcAddress(hModule, lpProcName);
    if (!ret) {
        char buf[MAXLINE];
        GetModuleFileName(hModule, buf, sizeof(buf));
        buf[MAXLINE - 1] = '\0';
        fail("can't find proc address for '%s' in module '%s'.", lpProcName, buf);
    }
    return ret;
}

int MessageBoxW_format(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, ...)
{
    va_list ap;
    va_start(ap, uType);
    int ret;
    wchar_t buf[MAXLINE];
    vsnwprintf(buf, MAXLINE, lpText, ap);
    ret = MessageBoxW(hWnd, buf, lpCaption, uType);
    va_end(ap);
    return ret;
}

int MessageBoxW_utf8format(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, ...)
{
    va_list ap;
    va_start(ap, uType);
    wchar_t *text = NULL, *caption = NULL;
    int ret;
    char buf[MAXLINE];
    vsnprintf(buf, MAXLINE, lpText, ap);
    cs2wcs_managed(buf, CP_UTF8, &text);
    cs2wcs_managed(lpCaption, CP_UTF8, &caption);
    ret = MessageBoxW(hWnd, text, caption, uType);
    free(text);
    free(caption);
    va_end(ap);
    return ret;
}

void NORETURN die(int status)
{
    TerminateProcess(GetCurrentProcess(), status);
    while (1);
}

