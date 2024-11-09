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
void ShowPleaseWaitDlg(CWnd *fawnd, LPCTSTR msg, bool busy, float progress, bool (*cancelfunc)(), bool redraw)
{
	bool first = false;
	if (!waitdlg) {
		waitdlg = new CPleaseWaitDlg;
		waitdlg->Create(IDD_PLEASEWAIT, fawnd);
		waitdlg->m_WaitProgress.SetRange32(0, (1 << 24));
		first = true;
	}

	if (progress >= 0 || cancelfunc) {
		waitdlg->m_WaitMessage1.ShowWindow(SW_HIDE);
		if (msg) waitdlg->m_WaitMessage2.SetWindowText(msg);
		waitdlg->m_WaitMessage2.ShowWindow(SW_SHOW);
	} else {
		waitdlg->m_WaitMessage2.ShowWindow(SW_HIDE);
		if (msg) waitdlg->m_WaitMessage1.SetWindowText(msg);
		waitdlg->m_WaitMessage1.ShowWindow(SW_SHOW);
	}

	if (progress >= 0) {
		waitdlg->m_WaitProgress.SetPos(progress * (1 << 24));
		waitdlg->m_WaitProgress.ShowWindow(SW_SHOW);
	} else {
		waitdlg->m_WaitProgress.ShowWindow(SW_HIDE);
	}

	if (waitdlg->cancelfunc != cancelfunc) {
		waitdlg->m_CancelBtn.EnableWindow(TRUE);
		waitdlg->cancelfunc = cancelfunc;
	}
	waitdlg->m_CancelBtn.ShowWindow(cancelfunc ? SW_SHOW : SW_HIDE);
	
	if (first) {
		waitdlg->ShowWindow(SW_SHOW);
		if (waitdlg->GetParent()) {
			waitdlg->GetParent()->BeginModalState();
		}
	}

	if (waitdlg->busy != busy) {
		if (busy) {
			waitdlg->BeginWaitCursor();
		} else {
			waitdlg->EndWaitCursor();
		}
		waitdlg->busy = busy;
	}

	if (redraw) {
		waitdlg->RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	DoEvents();

	if (busy) {
		waitdlg->RestoreWaitCursor();
	}
}
CPleaseWaitDlg *GetPleaseWaitDlg()
{
	return waitdlg;
}
void DestroyPleaseWaitDlg(CWnd *fawnd)
{
	if (waitdlg) {
		if (waitdlg->busy) {
			waitdlg->EndWaitCursor();
		}
		if (waitdlg->GetParent()) {
			waitdlg->GetParent()->EndModalState();
		}
		waitdlg->DestroyWindow();
		delete waitdlg;
		waitdlg = NULL;
		if (fawnd) {
			fawnd->RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
}


CPleaseWaitDlg::CPleaseWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPleaseWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPleaseWaitDlg)
	//}}AFX_DATA_INIT
	cancelfunc = NULL;
	busy = false;
}


void CPleaseWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPleaseWaitDlg)
	DDX_Control(pDX, IDC_WAITMESSAGE2, m_WaitMessage2);
	DDX_Control(pDX, IDC_WAITMESSAGE1, m_WaitMessage1);
	DDX_Control(pDX, IDCANCEL, m_CancelBtn);
	DDX_Control(pDX, IDC_WAITPROGRESS, m_WaitProgress);
	//}}AFX_DATA_MAP
}


void CPleaseWaitDlg::OnOK()
{
}

void CPleaseWaitDlg::OnCancel()
{
	if (cancelfunc && cancelfunc()) {
		m_CancelBtn.EnableWindow(FALSE);
	}
}

BEGIN_MESSAGE_MAP(CPleaseWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CPleaseWaitDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPleaseWaitDlg message handlers
