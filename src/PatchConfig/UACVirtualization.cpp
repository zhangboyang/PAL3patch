#include "stdafx.h"

typedef BOOL (WINAPI *OpenProcessToken_funcptr_t)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI *SetTokenInformation_funcptr_t)(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength);

#define MyTokenVirtualizationEnabled ((TOKEN_INFORMATION_CLASS) 24)

static OpenProcessToken_funcptr_t MyOpenProcessToken = NULL;
static SetTokenInformation_funcptr_t MySetTokenInformation = NULL;

static int SetUACVirtualization(bool en)
{
	HANDLE token;
	DWORD dwVal = en ? 1 : 0;
	int ret = 0;

	if (MyOpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &token)) {
		if (MySetTokenInformation(token, MyTokenVirtualizationEnabled, &dwVal, sizeof(dwVal))) {
			ret = 1;
		}
		CloseHandle(token);
	}
	return ret;
}

static int InitSetUACVirtualization()
{
	// flag  < 0  -  error occured
	// flag == 0  -  haven't initialized
	// flag  > 0  -  successfully initialized
	static int flag = 0;

	// check status
	if (flag != 0) return flag;

	// load library
	HMODULE hADVAPI32 = GetModuleHandle(_T("ADVAPI32.DLL"));
	if (!hADVAPI32) hADVAPI32 = LoadLibrary(_T("ADVAPI32.DLL"));

	// import functions
	if (hADVAPI32) {
		MyOpenProcessToken = (OpenProcessToken_funcptr_t) GetProcAddress(hADVAPI32, "OpenProcessToken");
		MySetTokenInformation = (SetTokenInformation_funcptr_t) GetProcAddress(hADVAPI32, "SetTokenInformation");
	}

	if (hADVAPI32 && MyOpenProcessToken && MySetTokenInformation) {
		return flag = 1;
	} else {
		return flag = -1;
	}
}

int TrySetUACVirtualization(bool en)
{
	if (InitSetUACVirtualization() > 0) {
		return SetUACVirtualization(en);
	} else {
		return 0;
	}
}

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
