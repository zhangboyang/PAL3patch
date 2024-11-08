// PatchConfig.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigApp

BEGIN_MESSAGE_MAP(CPatchConfigApp, CWinApp)
	//{{AFX_MSG_MAP(CPatchConfigApp)
	//}}AFX_MSG
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigApp construction

CPatchConfigApp::CPatchConfigApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPatchConfigApp object

CPatchConfigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigApp initialization

static int AcquireGameMutex()
{
	HANDLE hMutex;
	DWORD dwWaitResult;

	hMutex = CreateMutexA(NULL, FALSE, PATCH_MUTEXNAME);
	if (hMutex == NULL) goto fail;

	dwWaitResult = WaitForSingleObject(hMutex, 100);
	if (dwWaitResult != WAIT_OBJECT_0 && dwWaitResult != WAIT_ABANDONED) goto fail;

	return 1;
fail:
	MessageBox(NULL, STRTABLE(IDS_NOMUTEX), STRTABLE(IDS_NOMUTEX_TITLE), MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	return 0;
}

static int CheckCOMCTL32()
{
	DWORD dwMajorVersion = 0;

	HMODULE hCOMCTL32 = LoadLibrary(_T("comctl32.dll"));
	if (hCOMCTL32) {
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hCOMCTL32, "DllGetVersion");
		if (pDllGetVersion) {
			DLLVERSIONINFO dvi;
			HRESULT hr;
			memset(&dvi, 0, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			hr = (*pDllGetVersion)(&dvi);
			if (SUCCEEDED(hr)) {
				dwMajorVersion = dvi.dwMajorVersion;
			}
		}
        FreeLibrary(hCOMCTL32);
	}

	if (dwMajorVersion >= 5) {
		return 1;
	} else {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_BADCOMCTL32), STRTABLE(IDS_BADCOMCTL32_TITLE), MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		return 0;
	}
}

static int VerifyPatchFiles()
{
	const int max_show = 10;
	int cnt = 0;
	CString buf;
	const char **ptr;
	for (ptr = pFileHash; *ptr; ptr += 2) {
		const char *filename = ptr[0];
		const char *hashstr = ptr[1];
		SHA1Hash filehash;
		if (GetFileSHA1(filename, &filehash) && filehash == SHA1Hash::fromhex(hashstr)) {
			continue;
		}
		if (cnt <= max_show) {
			buf += _T("  ");
			buf += cnt < max_show ? filename : "...";
			buf += _T("\n");
		}
		cnt++;
	}
	if (cnt > 0) {
		CString msg;
		msg.Format(IDS_CORRUPTFILE, (LPCTSTR) buf);
		if (GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CORRUPTFILE_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) == IDNO) {
			return 0;
		}
	}
	return 1;
}

static int LoadConfigData()
{
	while (!TryLoadConfigData()) {
		if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CORRUPTCONFIG), STRTABLE(IDS_CORRUPTCONFIG_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) == IDYES) {
			TryRebuildConfigFile();
		} else {
			return 0;
		}
	}
	return 1;
}

BOOL CPatchConfigApp::InitInstance()
{
	// Standard initialization

	TrySetUACVirtualization(true);

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	if (!AcquireGameMutex()) goto err;

	if (!CheckDX90SDKVersion()) goto err;

	LoadConfigDescList(STRTABLE(IDS_CONFIGDESCLIST));

#ifdef BUILD_FOR_PAL3A
	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGCHECKBADFILES));
	CheckBadFilesForPAL3A();
#endif

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGVERIFY));
	if (!VerifyPatchFiles()) goto err;

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGLOADCFG));
	if (!LoadConfigData()) goto err;

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGENUMD3D));
	if (!InitD3DEnumeration()) goto err;

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGCHECKSYSTEM));
	if (!CheckCOMCTL32()) goto err;

	DestroyPleaseWaitDlg();

	// use block to surround dlg
	{
		CPatchConfigDlg dlg;
		// leave m_pMainWnd NULL, or we can't popup MessageBox after DoModal()
		//m_pMainWnd = &dlg;
		dlg.DoModal();

		CleanupD3DEnumeration();

		// Since the dialog has been closed, return FALSE so that we exit the
		//  application, rather than start the application's message pump.
		return FALSE;
	}

err:
	DestroyPleaseWaitDlg();
	CleanupD3DEnumeration();
	return FALSE;
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
