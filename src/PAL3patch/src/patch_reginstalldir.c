#include "common.h"

static void GetUACVirtualizedCurrentDirectory(LPCSTR testfile, LPSTR out, DWORD outsz);

static void try_register_install_dir_for_PAL3A()
{
    char installdir[MAXLINE];
    GetUACVirtualizedCurrentDirectory("save\\savetest.tmp", installdir, sizeof(installdir));
    if (!installdir[0]) {
        DWORD r = GetCurrentDirectory(sizeof(installdir), installdir);
        if (r == 0 || r >= MAXLINE) return;
    }
    int l = strlen(installdir);
    while (l > 0 && (installdir[l - 1] == '\\' || installdir[l - 1] == '/')) l--;
    installdir[l] = 0;
    
    const char *gametype;
    switch (game_locale) {
        case GAME_LOCALE_CHS: gametype = "CH"; break;
        case GAME_LOCALE_CHT: gametype = "TW"; break;
        default: return;
    }
    
    char subkey[MAXLINE];
    snprintf(subkey, sizeof(subkey), "SOFTWARE\\SOFTSTAR\\PAL3\\%s", gametype);
    
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, "Install", 0, REG_SZ, (CONST BYTE *) installdir, strlen(installdir) + 1);
        RegCloseKey(hKey);
    }
}

MAKE_PATCHSET(reginstalldir)
{
    try_register_install_dir_for_PAL3A();
}





//
//  below code are copied from PatchConfig
//
//  these macros are added for compatibility
//  because these macros are dirty, it must be put at the end of file
//


#undef TCHAR
#undef LPTSTR
#undef LPCTSTR
#undef _T
#undef _tcscmp
#undef _tcslen
#undef _tcscat
#undef _tcscpy
#undef _tfopen
#undef _tunlink
#define TCHAR char
#define LPTSTR LPSTR
#define LPCTSTR LPCSTR
#define _T(x) x
#define _tcscmp strcmp
#define _tcslen strlen
#define _tcscat strcat
#define _tcscpy strcpy
#define _tfopen fopen
#define _tunlink _unlink
#define InitSetUACVirtualization() 1

//
// copied from PatchConfig/UACVirtualization.cpp
//

void GetUACVirtualizedCurrentDirectory(LPCTSTR testfile, LPTSTR out, DWORD outsz)
{
	// if succeed, return buffer always ends with "\"
	// if failed or no-UAC, return buffer contains empty string

#define BUFLEN MAXLINE
#define UACTESTFILE _T("uactest.tmp")
#define UACSTOREROOT _T("%LOCALAPPDATA%\\VirtualStore")

	TCHAR curdir[BUFLEN], storedir[BUFLEN];
	SYSTEMTIME now;
#define TESTLEN 32
	char wdata[TESTLEN + 1], rdata[TESTLEN];
	FILE *wfp = NULL, *rfp = NULL;
	int wflag = 0;
	DWORD r;
	size_t l;

	if (!testfile) testfile = UACTESTFILE;

	// early test for no-UAC
	if (InitSetUACVirtualization() < 0) goto fail;

	// construct VirtualStore path
	r = ExpandEnvironmentStrings(UACSTOREROOT, storedir, BUFLEN);
	if (r == 0 || r > BUFLEN || _tcscmp(UACSTOREROOT, storedir) == 0) goto fail;

	// get current directory
	r = GetCurrentDirectory(BUFLEN, curdir);
	if (r == 0 || r >= BUFLEN) goto fail;
	if (!(_T('A') <= curdir[0] && curdir[0] <= _T('Z')) && !(_T('a') <= curdir[0] && curdir[0] <= _T('z'))) goto fail;
	if (curdir[1] != _T(':') || curdir[2] != _T('\\')) goto fail;

	// append curdir to storedir, without "X:"
	if (_tcslen(storedir) + _tcslen(curdir + 2) >= BUFLEN) goto fail;
	_tcscat(storedir, curdir + 2);

	// add slash if needed
	l = _tcslen(storedir);
	if (l && storedir[l - 1] != _T('\\')) {
		if (l + 1 >= BUFLEN) goto fail;
		storedir[l] = _T('\\');
		storedir[l + 1] = 0;
	}

	// construct mapped test file path in curdir
	_tcscpy(curdir, storedir);
	if (_tcslen(curdir) + _tcslen(testfile) >= BUFLEN) goto fail;
	_tcscat(curdir, testfile);

	// generate test data
	GetLocalTime(&now);
	sprintf(wdata, "%04x%04x%04x%04x%04x%04x%04x%04x", now.wYear, now.wMonth, now.wDayOfWeek, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

	// write test file
	wfp = _tfopen(testfile, _T("wb"));
	if (!wfp) goto fail;
	wflag = 1;
	if (fwrite(wdata, 1, TESTLEN, wfp) != TESTLEN) goto fail;
	fclose(wfp); wfp = NULL;

	// read test file
	rfp = _tfopen(curdir, _T("rb"));
	if (!rfp) goto fail;
	if (fread(rdata, 1, TESTLEN, rfp) != TESTLEN) goto fail;
	fclose(rfp); rfp = NULL;

	// compare test data
	if (memcmp(wdata, rdata, TESTLEN) != 0) goto fail;

	// write to result buffer
	if (_tcslen(storedir) >= outsz) goto fail;
	_tcscpy(out, storedir);

done:
	// free resource
	if (wfp) fclose(wfp);
	if (rfp) fclose(rfp);
	if (wflag) _tunlink(testfile);
	return;

fail:
	// clear result buffer
	if (outsz) *out = 0;
	goto done;
}
