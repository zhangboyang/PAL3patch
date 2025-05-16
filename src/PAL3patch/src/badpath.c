#include "common.h"

static size_t acplen(const wchar_t *wstr)
{
    char *s = wcs2cs_alloc(wstr, CP_ACP);
    size_t len = strlen(s);
    free(s);
    return len;
}

static int acptest(const wchar_t *wstr)
{
    char *s = wcs2cs_alloc(wstr, CP_ACP);
    wchar_t *ws = cs2wcs_alloc(s, CP_ACP);
    int ret = wcscmp(wstr, ws) == 0;
    free(ws);
    free(s);
    return ret;
}

void check_badpath()
{
    wchar_t path[MAXLINE];
    DWORD ret = GetCurrentDirectoryW(MAXLINE, path);

#define LONGPATH_LIMIT 200
    if (ret == 0 || ret >= MAXLINE || wcslen(path) > LONGPATH_LIMIT || acplen(path) > LONGPATH_LIMIT) {
        if (ret == 0 && is_win9x() && !is_kernelex()) return;
        MessageBoxW(NULL, wstr_pathtoolong_text, wstr_pathtoolong_title, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
        return;
    }

    if (!acptest(path)) {
        MessageBoxW(NULL, wstr_badpath_text, wstr_badpath_title, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
        return;
    }
}
