#include "stdafx.h"

int is_win9x()
{
    LPOSVERSIONINFO osvi = GetVersionEx_cached();
    return osvi && osvi->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
}

int is_winxp_or_later()
{
    LPOSVERSIONINFO osvi = GetVersionEx_cached();
    return osvi && osvi->dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi->dwMajorVersion > 5 || (osvi->dwMajorVersion == 5 && osvi->dwMinorVersion >= 1));
}

HANDLE acquire_mutex(LPCSTR lpName, DWORD dwMilliseconds)
{
	HANDLE hMutex;
	DWORD dwWaitResult;

	hMutex = CreateMutexA(NULL, FALSE, lpName);
	if (hMutex == NULL) goto fail;

	dwWaitResult = WaitForSingleObject(hMutex, dwMilliseconds);
	if (dwWaitResult != WAIT_OBJECT_0 && dwWaitResult != WAIT_ABANDONED) goto fail;

	return hMutex;

fail:
    if (hMutex) CloseHandle(hMutex);
    return NULL;
}

void release_mutex(HANDLE hMutex)
{
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

LPOSVERSIONINFO GetVersionEx_cached()
{
    static OSVERSIONINFO osvi;
    static BOOL ret = 0;
    if (!ret) {
        memset(&osvi, 0, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        ret = GetVersionEx(&osvi);
    }
    return ret ? &osvi : NULL;
}

void DoEvents()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		AfxGetApp()->PumpMessage();
	}
}

void *Malloc(size_t n)
{
	void *ret = malloc(n);
	if (!ret) {
		die(1);
	}
	return ret;
}

__declspec(noreturn) void die(int status)
{
    TerminateProcess(GetCurrentProcess(), status);
    while (1);
}
