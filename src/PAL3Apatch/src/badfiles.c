#include "common.h"

void check_badfiles()
{
	const char *main_exe = "PAL3A.exe";
	static const char *bad_files[] = {
		"BugslayerUtil.dll",
		"CollidePackage.dll",
		"dbghelp.dll",
		"MSVCP60.DLL",
		"MSVCRTD.DLL",
		"PAL3A.pdb",
		NULL // EOF
	};

	struct cstr buf;
	const char **ptr;
	cstr_ctor(&buf);
	
	if (!file_exists(main_exe)) return;
	cstr_clear(&buf);
	for (ptr = bad_files; *ptr; ptr++) {
		if (file_exists(*ptr)) {
			cstr_strcat(&buf, "    ");
			cstr_strcat(&buf, *ptr);
			cstr_strcat(&buf, "\n");
		}
	}
	if (!cstr_empty(&buf)) {
		if (MessageBoxW_format(NULL, wstr_havebadfile_text, wstr_havebadfile_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1, cstr_getstr(&buf)) == IDYES) {
			cstr_clear(&buf);
			for (ptr = bad_files; *ptr; ptr++) {
				if (robust_unlink(*ptr) != 0 && errno != ENOENT) {
					cstr_strcat(&buf, "    ");
					cstr_strcat(&buf, *ptr);
					cstr_strcat(&buf, "\n");
				}
			}
			if (!cstr_empty(&buf)) {
				MessageBoxW_format(NULL, wstr_cantdelbadfile_text, wstr_cantdelbadfile_title, MB_ICONWARNING, cstr_getstr(&buf));
			}
		}
	}
}
