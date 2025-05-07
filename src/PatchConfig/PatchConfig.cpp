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

static void AcquireGameMutex()
{
	if (!acquire_mutex(PATCH_GAME_MUTEX, 100)) {
		MessageBox(NULL, STRTABLE(IDS_NOMUTEX), STRTABLE(IDS_NOMUTEX_TITLE), MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		die(0);
	}
}

static void CheckCOMCTL32()
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

	if (dwMajorVersion < 5) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_BADCOMCTL32), STRTABLE(IDS_BADCOMCTL32_TITLE), MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		die(0);
	}
}

static void CheckBadPath()
{
	int game_locale = detect_game_locale();
	if (game_locale < 0) return;
	unsigned system_codepage = GetACP();
	if (game_locale == 0 && system_codepage == 936) return;
	if (game_locale == 1 && system_codepage == 950) return;

    TCHAR buf[MAXLINE];
    if (GetModuleFileName(NULL, buf, MAXLINE) != 0) {
        wchar_t *ptr;
        for (ptr = buf; *ptr; ptr++) {
            if (*ptr < 0x20 || *ptr > 0x7E) {
                break;
            }
        }
        if (*ptr) {
            GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_BADPATH), STRTABLE(IDS_BADPATH_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
        }
    }
}

static void VerifyPatchFiles()
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
			die(0);
		}
	}
}

static void LoadConfigData()
{
	while (!TryLoadConfigData()) {
		if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CORRUPTCONFIG), STRTABLE(IDS_CORRUPTCONFIG_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) == IDYES) {
			TryRebuildConfigFile();
		} else {
			die(0);
		}
	}
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

	AcquireGameMutex();

	LoadConfigDescList(STRTABLE(IDS_CONFIGDESCLIST));

#ifdef BUILD_FOR_PAL3A
	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGCHECKBADFILES));
	CheckBadFilesForPAL3A();
#endif

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGVERIFY));
	VerifyPatchFiles();

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGLOADCFG));
	LoadConfigData();

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGCHECKSYSTEM));
	CheckCOMCTL32();
	CheckBadPath();

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGENUMD3D));
	FirstD3DEnumeration();

	DestroyPleaseWaitDlg();

	// use block to surround dlg
	{
		CPatchConfigDlg dlg;
		// leave m_pMainWnd NULL, or we can't popup MessageBox after DoModal()
		//m_pMainWnd = &dlg;
		dlg.DoModal();

		// Since the dialog has been closed, return FALSE so that we exit the
		//  application, rather than start the application's message pump.
		return FALSE;
	}
}
