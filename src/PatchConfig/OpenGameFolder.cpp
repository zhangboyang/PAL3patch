#include "stdafx.h"


static void OpenGameFolder(CPatchConfigDlg *dlg, LPCTSTR subdir)
{
	static TCHAR curdir[MAXLINE];
	static bool curdir_flag = false;

	TCHAR buf[MAXLINE];
	LPCTSTR openpath;
	CString msg;
	DWORD r;

	msg.Format(IDS_OPENGAMEFOLDERINPROGRESS, subdir);
	ShowPleaseWaitDlg(dlg, msg);

	if (!curdir_flag) {
		GetUACVirtualizedCurrentDirectory(curdir, MAXLINE);
		curdir_flag = true;
	}

	_tcscpy(buf, curdir);
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
	OpenGameFolder(dlg, _T("save"));
}

void OpenSnapFolder(CPatchConfigDlg *dlg)
{
	dlg->SetTopMost(false);
	create_dir("snap");
	OpenGameFolder(dlg, _T("snap"));
}
