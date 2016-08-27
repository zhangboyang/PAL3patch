// PleaseWaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include <assert.h>
#include "PatchConfig.h"
#include "PleaseWaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPleaseWaitDlg dialog

static CPleaseWaitDlg *waitdlg = NULL;
void ShowPleaseWaitDlg(LPCTSTR msg)
{
	if (!waitdlg) {
		waitdlg = new CPleaseWaitDlg;
		waitdlg->Create(IDD_PLEASEWAIT);
		waitdlg->m_WaitMessage = CString(msg);
		waitdlg->UpdateData(FALSE);
		waitdlg->ShowWindow(SW_SHOW);
		waitdlg->RedrawWindow();
	} else {
		waitdlg->m_WaitMessage = CString(msg);
		waitdlg->UpdateData(FALSE);
		waitdlg->RedrawWindow();
	}
}
HWND GetWaitDlgHandle()
{
	return waitdlg ? waitdlg->m_hWnd : NULL;
}
void DestoryPleaseWaitDlg()
{
	if (waitdlg) {
		delete waitdlg;
		waitdlg = NULL;
	}
}


CPleaseWaitDlg::CPleaseWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPleaseWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPleaseWaitDlg)
	m_WaitMessage = _T("");
	//}}AFX_DATA_INIT
}


void CPleaseWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPleaseWaitDlg)
	DDX_Text(pDX, IDC_WAITMESSAGE, m_WaitMessage);
	//}}AFX_DATA_MAP
}


void CPleaseWaitDlg::OnOK()
{
}

void CPleaseWaitDlg::OnCancel()
{
}

BEGIN_MESSAGE_MAP(CPleaseWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CPleaseWaitDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPleaseWaitDlg message handlers
