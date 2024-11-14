#include "stdafx.h"


static BOOL CALLBACK SetForegroundIfMatched(HWND hwnd, LPARAM lParam)
{
	PROCESS_INFORMATION *pi = (PROCESS_INFORMATION *) lParam;
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == pi->dwProcessId && !GetWindow(hwnd, GW_OWNER) && IsWindowVisible(hwnd)) {
		if (SetForegroundWindow(hwnd)) {
			FLASHWINFO fwi;
			fwi.cbSize = sizeof(fwi);
			fwi.hwnd = hwnd;
			fwi.dwFlags = FLASHW_CAPTION;
			fwi.uCount = 3;
			fwi.dwTimeout = 80;
			FlashWindowEx(&fwi);
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
				EnumWindows(SetForegroundIfMatched, (LPARAM) &pi);
			}
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	DestroyPleaseWaitDlg(dlg);

	dlg->SetTopMost(true);
	dlg->SetForegroundWindow();
}
