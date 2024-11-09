#if !defined(AFX_PLEASEWAITDLG_H__6ED11260_F47F_4DE6_8CAD_2D4FE8973675__INCLUDED_)
#define AFX_PLEASEWAITDLG_H__6ED11260_F47F_4DE6_8CAD_2D4FE8973675__INCLUDED_


#include "Resource.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PleaseWaitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPleaseWaitDlg dialog

class CPleaseWaitDlg : public CDialog
{
// Construction
public:
	CPleaseWaitDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPleaseWaitDlg)
	enum { IDD = IDD_PLEASEWAIT };
	CStatic	m_WaitMessage2;
	CStatic	m_WaitMessage1;
	CButton	m_CancelBtn;
	CProgressCtrl	m_WaitProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPleaseWaitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool (*cancelfunc)();
	bool busy;
	friend void ShowPleaseWaitDlg(CWnd *fawnd, LPCTSTR msg, bool busy, float progress, bool (*cancelfunc)(), bool redraw);
	friend void DestroyPleaseWaitDlg(CWnd *fawnd);
	
	virtual void CPleaseWaitDlg::OnOK();
	virtual void CPleaseWaitDlg::OnCancel();
	// Generated message map functions
	//{{AFX_MSG(CPleaseWaitDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


extern void ShowPleaseWaitDlg(CWnd *fawnd, LPCTSTR msg, bool busy = true, float progress = -1.0f, bool (*cancelfunc)() = NULL, bool redraw = true);
extern CPleaseWaitDlg *GetPleaseWaitDlg();
extern void DestroyPleaseWaitDlg(CWnd *fawnd = NULL);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLEASEWAITDLG_H__6ED11260_F47F_4DE6_8CAD_2D4FE8973675__INCLUDED_)
