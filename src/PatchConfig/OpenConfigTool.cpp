#include "stdafx.h"

static const wchar_t wtitle_CHS[] = _T("环境配置");
static const wchar_t wtitle_CHT[] = _T("設定程式");

static char *title_CHS = NULL;
static char *title_CHT = NULL;

static BOOL CALLBACK SetForegroundIfMatched(HWND hwnd, LPARAM lParam)
{
	char buf[4096];
	if (!GetWindow(hwnd, GW_OWNER) && IsWindowVisible(hwnd) && GetWindowTextA(hwnd, buf, sizeof(buf)) && (strcmp(buf, title_CHS) == 0 || strcmp(buf, title_CHT) == 0)) {
		if (SetForegroundWindow(hwnd)) {
			if (!is_win9x()) {
				FLASHWINFO fwi;
				fwi.cbSize = sizeof(fwi);
				fwi.hwnd = hwnd;
				fwi.dwFlags = FLASHW_CAPTION;
				fwi.uCount = 3;
				fwi.dwTimeout = 80;
				FlashWindowEx(&fwi);
			}
			MessageBeep(MB_OK);
		}
		return FALSE;
	}
	return TRUE;
}

void OpenConfigTool(CPatchConfigDlg *dlg)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	dlg->SetTopMost(false);
	ShowPleaseWaitDlg(dlg, STRTABLE(IDS_OPENCONFIGTOOLINPROGRESS));

	if (!title_CHS) title_CHS = wcs2cs_alloc(wtitle_CHS, CP_ACP);
	if (!title_CHT) title_CHT = wcs2cs_alloc(wtitle_CHT, CP_ACP);

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	if (!CreateProcess(_T("config.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CANTOPENCONFIGTOOL), STRTABLE(IDS_CANTOPENCONFIGTOOL_TITLE), MB_ICONERROR);
	} else {
		ShowPleaseWaitDlg(dlg, STRTABLE(IDS_OPENCONFIGTOOLWAITFINISH), false);
		while (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 100, QS_ALLEVENTS) != WAIT_OBJECT_0) {
			DoEvents();
			if (GetPleaseWaitDlg()->m_hWnd == GetForegroundWindow()) {
				EnumThreadWindows(pi.dwThreadId, SetForegroundIfMatched, 0);
			}
		}
		DWORD dwExitCode;
		if (!GetExitCodeProcess(pi.hProcess, &dwExitCode) || (dwExitCode != 0 && dwExitCode != 27)) {
			GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CONFIGTOOLERROR), STRTABLE(IDS_CONFIGTOOLERROR_TITLE), MB_ICONERROR);
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	DestroyPleaseWaitDlg(dlg);
	dlg->SetTopMost(true);
	dlg->SetForegroundWindow();
}
