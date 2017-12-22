#include "stdafx.h"

void OpenConfigTool(CPatchConfigDlg *dlg)
{
	DWORD r;

	ShowPleaseWaitDlg(dlg, STRTABLE(IDS_OPENCONFIGTOOLINPROGRESS));

	r = (DWORD) ShellExecute(dlg->GetSafeHwnd(), _T("open"), _T("config.exe"), NULL, NULL, SW_SHOWNORMAL);
	if (r <= 32) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CANTOPENCONFIGTOOL), STRTABLE(IDS_CANTOPENCONFIGTOOL_TITLE), MB_ICONERROR);
	}

	DestroyPleaseWaitDlg();
}
