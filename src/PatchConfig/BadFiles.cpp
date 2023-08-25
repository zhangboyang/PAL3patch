#include "stdafx.h"

#ifdef BUILD_FOR_PAL3A
void CheckBadFilesForPAL3A()
{
	const char *main_exe = "PAL3A.exe";
	static const char *bad_files[] = {
#ifndef _DEBUG
		"BugslayerUtil.dll",
		"CollidePackage.dll",
		"dbghelp.dll",
		"MSVCP60.DLL",
		"MSVCRTD.DLL",
		"PAL3A.pdb",
#endif
		NULL // EOF
	};

	CString buf;
	const char **ptr;
	
	if (!file_exists(main_exe)) return;
	buf.Empty();
	for (ptr = bad_files; *ptr; ptr++) {
		if (file_exists(*ptr)) {
			buf += "    ";
			buf += *ptr;
			buf += "\n";
		}
	}
	if (!buf.IsEmpty()) {
		CString msg;
		msg.Format(IDS_HAVEBADFILE, (LPCTSTR) buf);
		if (GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_HAVEBADFILE_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1 | MB_TOPMOST | MB_SETFOREGROUND) == IDYES) {
			buf.Empty();
			for (ptr = bad_files; *ptr; ptr++) {
				if (robust_unlink(*ptr) != 0 && errno != ENOENT) {
					buf += "    ";
					buf += *ptr;
					buf += "\n";
				}
			}
			if (!buf.IsEmpty()) {
				msg.Format(IDS_CANTDELBADFILE, buf);
				GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CANTDELBADFILE_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}
	}
}
#endif