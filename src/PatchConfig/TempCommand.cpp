#include "stdafx.h"

static bool OperateTempFile(CWnd *fawnd, bool wr, bool (*op)(FILE *))
{
	HANDLE hMutex = acquire_mutex(PATCH_TEMP_MUTEX, 100);
	if (!hMutex) {
		fawnd->MessageBox(STRTABLE(IDS_TEMP_NOMUTEX), STRTABLE(IDS_TEMP_NOMUTEX_TITLE), MB_ICONERROR);
		return false;
	}
	bool success = false;
	FILE *fp = wr ? robust_fopen(PATCH_TEMP_IN, "w") : robust_fopen(PATCH_TEMP_OUT, "r");
	if (fp) {
		success = op(fp);
		fclose(fp);
	}
	release_mutex(hMutex);
	return success;
}

static void CleanupTempFile()
{
	HANDLE hMutex = acquire_mutex(PATCH_TEMP_MUTEX, 100);
	if (!hMutex) return;
	const char *temp[2] = { PATCH_TEMP_IN, PATCH_TEMP_OUT };
	robust_delete(temp, 2);
	release_mutex(hMutex);
}

static bool InvokeGameEXE()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	if (!CreateProcess(GAME_EXE, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		return false;
	} else {
		while (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 100, QS_ALLEVENTS) != WAIT_OBJECT_0) {
			DoEvents();
		}
		bool success = false;
		DWORD dwExitCode;
		if (GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
			success = dwExitCode == 0;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return success;
	}
}

bool InvokeTempCommand(CWnd *fawnd, bool (*writer)(FILE *), bool (*reader)(FILE *))
{
	bool success = OperateTempFile(fawnd, true, writer) && InvokeGameEXE() && OperateTempFile(fawnd, false, reader);
	CleanupTempFile();
	return success;
}
