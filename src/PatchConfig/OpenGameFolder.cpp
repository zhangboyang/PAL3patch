#include "stdafx.h"


static void OpenGameFolder(CPatchConfigDlg *dlg, LPCTSTR subdir)
{
	TCHAR buf[MAXLINE];
	LPCTSTR openpath;
	CString msg;
	DWORD r;

	msg.Format(IDS_OPENGAMEFOLDERINPROGRESS, subdir);
	ShowPleaseWaitDlg(dlg, msg);

	GetUACVirtualizedCurrentDirectory(buf, MAXLINE);
	if (_tcslen(buf) + _tcslen(subdir) >= MAXLINE) {
		openpath = subdir;
	} else {
		_tcscat(buf, subdir);
		openpath = buf;
	}
	
	r = (DWORD) ShellExecute(NULL, _T("explore"), openpath, NULL, NULL, SW_SHOWNORMAL);
	if (r <= 32) {
		msg.Format(IDS_CANTOPENGAMEFOLDER, subdir);
		GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CANTOPENGAMEFOLDER_TITLE), MB_ICONERROR);
	}

	DestroyPleaseWaitDlg();
}



void OpenSaveFolder(CPatchConfigDlg *dlg)
{
	OpenGameFolder(dlg, _T("save"));
}

void OpenSnapFolder(CPatchConfigDlg *dlg)
{
	OpenGameFolder(dlg, _T("snap"));
}
