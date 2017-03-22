#include "stdafx.h"

typedef BOOL (WINAPI *OpenProcessToken_funcptr_t)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI *SetTokenInformation_funcptr_t)(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength);

#define MyTokenVirtualizationEnabled ((TOKEN_INFORMATION_CLASS) 24)

static OpenProcessToken_funcptr_t MyOpenProcessToken = NULL;
static SetTokenInformation_funcptr_t MySetTokenInformation = NULL;

static int SetUACVirtualization(BOOL enabled)
{
	HANDLE token;
	DWORD dwVal = enabled ? 1 : 0;
	int ret = 0;

	if (SUCCEEDED(MyOpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &token))) {
		if (SUCCEEDED(MySetTokenInformation(token, MyTokenVirtualizationEnabled, &dwVal, sizeof(dwVal)))) {
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

int TrySetUACVirtualization(BOOL enabled)
{
	if (InitSetUACVirtualization() > 0) {
		return SetUACVirtualization(enabled);
	} else {
		return 0;
	}
}
