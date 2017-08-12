// PleaseWaitDlg.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPleaseWaitDlg dialog

static CPleaseWaitDlg *waitdlg = NULL;
void ShowPleaseWaitDlg(CWnd* fawnd, LPCTSTR msg)
{
	if (!waitdlg) {
		waitdlg = new CPleaseWaitDlg;
		waitdlg->Create(IDD_PLEASEWAIT, fawnd);
		waitdlg->m_WaitMessage = CString(msg);
		waitdlg->UpdateData(FALSE);
		waitdlg->ShowWindow(SW_SHOW);
		waitdlg->BeginWaitCursor();
	} else {
		waitdlg->m_WaitMessage = CString(msg);
		waitdlg->UpdateData(FALSE);
	}

	waitdlg->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	if (waitdlg->GetParent()) {
		waitdlg->GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		waitdlg->GetParent()->BeginModalState();
	}
}
CPleaseWaitDlg *GetPleaseWaitDlg()
{
	return waitdlg;
}
void DestroyPleaseWaitDlg()
{
	if (waitdlg) {
		waitdlg->EndWaitCursor();
		if (waitdlg->GetParent()) {
			waitdlg->GetParent()->EndModalState();
			waitdlg->GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
		waitdlg->DestroyWindow();
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
