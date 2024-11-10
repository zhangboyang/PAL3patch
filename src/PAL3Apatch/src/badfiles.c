#include "common.h"

static int is_winxp_or_later()
{
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    return GetVersionEx(&osvi) && osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi.dwMajorVersion > 5 || (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1));
}

void check_badfiles()
{
	const char *main_exe = "PAL3A.exe";
	static const char *bad_files_retro[] = {
		"BugslayerUtil.dll",
		"CollidePackage.dll",
		"dbghelp.dll",
		"MSVCRTD.DLL",
		"PAL3A.pdb",
		NULL // EOF
	};
	static const char *bad_files_modern[] = {
		"BugslayerUtil.dll",
		"CollidePackage.dll",
		"dbghelp.dll",
		"MSVCP60.DLL",
		"MSVCRTD.DLL",
		"PAL3A.pdb",
		NULL // EOF
	};
	
	if (!file_exists(main_exe)) return;
	
	const char **bad_files = is_winxp_or_later() ? bad_files_modern : bad_files_retro;
	
	struct cstr buf;
	const char **ptr;
	cstr_ctor(&buf);
	
	cstr_clear(&buf);
	for (ptr = bad_files; *ptr; ptr++) {
		if (file_exists(*ptr)) {
			cstr_strcat(&buf, "    ");
			cstr_strcat(&buf, *ptr);
			cstr_strcat(&buf, "\n");
		}
	}
	if (!cstr_empty(&buf)) {
		if (MessageBoxW_format(NULL, wstr_havebadfile_text, wstr_havebadfile_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1 | MB_TOPMOST | MB_SETFOREGROUND, cstr_getstr(&buf)) == IDYES) {
			cstr_clear(&buf);
			for (ptr = bad_files; *ptr; ptr++) {
				if (robust_unlink(*ptr) != 0 && errno != ENOENT) {
					cstr_strcat(&buf, "    ");
					cstr_strcat(&buf, *ptr);
					cstr_strcat(&buf, "\n");
				}
			}
			if (!cstr_empty(&buf)) {
				MessageBoxW_format(NULL, wstr_cantdelbadfile_text, wstr_cantdelbadfile_title, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND, cstr_getstr(&buf));
			}
		}
	}
	
	cstr_dtor(&buf);
}
