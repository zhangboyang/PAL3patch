#include "common.h"

static size_t acplen(const wchar_t *ws)
{
    char *s = wcs2cs_alloc(ws, CP_ACP);
    size_t l = strlen(s);
    free(s);
    return l;
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

    if (system_codepage == target_codepage || system_codepage == CP_UTF8) {
        return;
    }

    wchar_t *ptr;
    for (ptr = path; *ptr; ptr++) {
        if (*ptr < 0x20 || *ptr > 0x7E) {
            break;
        }
    }
    if (*ptr) {
        MessageBoxW(NULL, wstr_pathnotenglish_text, wstr_pathnotenglish_title, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
    }
}
