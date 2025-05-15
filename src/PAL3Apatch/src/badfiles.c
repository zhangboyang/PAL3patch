#include "common.h"

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
	int n;

	const char **ptr;
	struct cstr buf;
	cstr_ctor(&buf);
	
	cstr_clear(&buf);
	for (ptr = bad_files; *ptr; ptr++) {
		if (file_exists(*ptr)) {
			cstr_strcat(&buf, "    ");
			cstr_strcat(&buf, *ptr);
			cstr_strcat(&buf, "\n");
		}
	}
	n = ptr - bad_files;

	if (!cstr_empty(&buf)) {
		if (MessageBoxW_format(NULL, wstr_havebadfile_text, wstr_havebadfile_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1 | MB_TOPMOST | MB_SETFOREGROUND, cstr_getstr(&buf)) == IDYES) {
			if (!batch_delete(bad_files, n)) {
				cstr_clear(&buf);
				for (ptr = bad_files; *ptr; ptr++) {
					if (file_exists(*ptr)) {
						cstr_strcat(&buf, "    ");
						cstr_strcat(&buf, *ptr);
						cstr_strcat(&buf, "\n");
					}
				}
				MessageBoxW_format(NULL, wstr_cantdelbadfile_text, wstr_cantdelbadfile_title, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND, cstr_getstr(&buf));
			}
		}
	}
	
	cstr_dtor(&buf);
}
