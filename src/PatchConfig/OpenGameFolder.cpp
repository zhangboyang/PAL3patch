#include "stdafx.h"


static void OpenGameFolder(CPatchConfigDlg *dlg, LPCTSTR testfile, LPCTSTR subdir)
{
	TCHAR buf[MAXLINE];
	LPCTSTR openpath;
	CString msg;
	DWORD r;

	msg.Format(IDS_OPENGAMEFOLDERINPROGRESS, subdir);
	ShowPleaseWaitDlg(dlg, msg);

	GetUACVirtualizedCurrentDirectory(testfile, buf, MAXLINE);
	if (!buf[0]) GetUACVirtualizedCurrentDirectory(NULL, buf, MAXLINE);

	if (_tcslen(buf) + _tcslen(subdir) >= MAXLINE) {
		openpath = subdir;
	} else {
		_tcscat(buf, subdir);
		openpath = buf;
	}
	
	r = (DWORD) ShellExecute(dlg->GetSafeHwnd(), _T("explore"), openpath, NULL, NULL, SW_SHOWNORMAL);
	if (r <= 32) {
		msg.Format(IDS_CANTOPENGAMEFOLDER, subdir);
		GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CANTOPENGAMEFOLDER_TITLE), MB_ICONERROR);
	}

	DestroyPleaseWaitDlg(dlg);
}



void OpenSaveFolder(CPatchConfigDlg *dlg)
{
	dlg->SetTopMost(false);
	create_dir("save");
	OpenGameFolder(dlg, _T("save\\savetest.tmp"), _T("save"));
}

void OpenSnapFolder(CPatchConfigDlg *dlg)
{
	dlg->SetTopMost(false);
	create_dir("snap");
	OpenGameFolder(dlg, _T("snap\\snaptest.tmp"), _T("snap"));
}
