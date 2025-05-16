#include "stdafx.h"

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

void CheckBadPath()
{
    wchar_t path[MAXLINE];
    DWORD ret = GetCurrentDirectoryW(MAXLINE, path);

#define LONGPATH_LIMIT 200
    if (ret == 0 || ret >= MAXLINE || wcslen(path) > LONGPATH_LIMIT || acplen(path) > LONGPATH_LIMIT) {
        GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_PATHTOOLONG), STRTABLE(IDS_PATHTOOLONG_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
        return;
    }

    if (!acptest(path)) {
        GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_BADPATH), STRTABLE(IDS_BADPATH_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
        return;
    }
}
