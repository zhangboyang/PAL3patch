#include "stdafx.h"

#ifdef BUILD_FOR_PAL3A

static bool is_winxp_or_later()
{
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    return GetVersionEx(&osvi) && osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi.dwMajorVersion > 5 || (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1));
}

void CheckBadFilesForPAL3A()
{
	const char *main_exe = "PAL3A.exe";
	static const char *bad_files_retro[] = {
#ifndef _DEBUG
		"BugslayerUtil.dll",
		"CollidePackage.dll",
		"dbghelp.dll",
		"MSVCRTD.DLL",
		"PAL3A.pdb",
#endif
		NULL // EOF
	};
	static const char *bad_files_modern[] = {
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
	const char **bad_files;
	const char **ptr;
	
	if (!file_exists(main_exe)) return;

	bad_files = is_winxp_or_later() ? bad_files_modern : bad_files_retro;

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
