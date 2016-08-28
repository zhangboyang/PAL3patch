// PatchConfig.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PatchConfig.h"
#include "PatchConfigDlg.h"
#include "PleaseWaitDlg.h"
#include "ConfigData.h"
#include "HASH_SHA1.h"
#include "PatchVersionInfo.h"

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
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
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
		msg.Format(STRTABLE(IDS_CORRUPTFILE), fn);
		if (MessageBox(GetWaitDlgHandle(), msg, STRTABLE(IDS_CORRUPTFILE_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
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
		if (MessageBox(GetWaitDlgHandle(), STRTABLE(IDS_CORRUPTCONFIG), STRTABLE(IDS_CORRUPTCONFIG_TITLE), MB_ICONWARNING | MB_YESNO) == IDYES) {
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

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif



	CPatchConfigDlg dlg;



	ShowPleaseWaitDlg(STRTABLE(IDS_WAITINGVERIFY)); //Sleep(1000);
	if (!VerifyPatchFiles()) goto err;

	ShowPleaseWaitDlg(STRTABLE(IDS_WAITINGLOADCFG)); //Sleep(1000);
	if (!LoadConfigData()) goto err;

	ShowPleaseWaitDlg(STRTABLE(IDS_WAITINGENUMD3D)); //Sleep(1000);
	InitD3DEnumeration();

	DestoryPleaseWaitDlg();


	// leave m_pMainWnd NULL, or we can't popup MessageBox after DoModal()
	//m_pMainWnd = &dlg;
	dlg.DoModal();

	CleanupD3DEnumeration();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;

err:
	DestoryPleaseWaitDlg();
	CleanupD3DEnumeration();
	return FALSE;
}
