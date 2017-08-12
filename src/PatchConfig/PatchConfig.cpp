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


static int VerifyFileSHA1(const char *fn, const char *hash)
{
	char hashstr[SHA1_STR_SIZE];
	if (!GetFileSHA1(fn, hashstr) || stricmp(hashstr, hash) != 0) {
		CString msg;
		msg.Format(IDS_CORRUPTFILE, fn);
		if (GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CORRUPTFILE_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
			return 0;
		}
	}
	return 1;
}

static int VerifyPatchFiles()
{
	const char **ptr;
	for (ptr = pFileHash; *ptr; ptr += 2) {
		if (!VerifyFileSHA1(ptr[0], ptr[1])) {
			return 0;
		}
	}
	return 1;
}

static int LoadConfigData()
{
	while (!TryLoadConfigData()) {
		if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CORRUPTCONFIG), STRTABLE(IDS_CORRUPTCONFIG_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
			TryRebuildConfigFile();
		} else {
			return 0;
		}
	}
	return 1;
}

static void InitFolders()
{
	CreateDirectory(_T("save"), NULL);
    CreateDirectory(_T("snap"), NULL);
}




BOOL CPatchConfigApp::InitInstance()
{
	// Standard initialization

	TrySetUACVirtualization(TRUE);

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	if (!CheckDX90SDKVersion()) goto err;

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGVERIFY)); //Sleep(1000);
	if (!VerifyPatchFiles()) goto err;

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGLOADCFG)); //Sleep(1000);
	if (!LoadConfigData()) goto err;
	InitFolders();

	ShowPleaseWaitDlg(NULL, STRTABLE(IDS_WAITINGENUMD3D)); //Sleep(1000);
	InitD3DEnumeration();

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
