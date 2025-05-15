#include "stdafx.h"

static size_t acplen(const wchar_t *ws)
{
    char *s = wcs2cs_alloc(ws, CP_ACP);
    size_t l = strlen(s);
    free(s);
    return l;
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

	int game_locale = detect_game_locale();
	if (game_locale < 0) return;
	unsigned system_codepage = GetACP();
	if (game_locale == 0 && system_codepage == 936) return;
	if (game_locale == 1 && system_codepage == 950) return;
	if (system_codepage == CP_UTF8) return;

    wchar_t *ptr;
    for (ptr = path; *ptr; ptr++) {
        if (*ptr < 0x20 || *ptr > 0x7E) {
            break;
        }
    }
    if (*ptr) {
        GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_PATHNOTENGLISH), STRTABLE(IDS_PATHNOTENGLISH_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
    }
}
