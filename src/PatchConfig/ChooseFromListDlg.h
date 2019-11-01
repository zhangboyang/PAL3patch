#if !defined(AFX_CHOOSEFROMLISTDLG_H__73436BF1_272B_4800_9B03_501709644D43__INCLUDED_)
#define AFX_CHOOSEFROMLISTDLG_H__73436BF1_272B_4800_9B03_501709644D43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseFromListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseFromListDlg dialog

class CChooseFromListDlg : public CDialog
{
// Construction
public:
	CChooseFromListDlg(CWnd* pParent = NULL);   // standard constructor

	ConfigDescOptionListEnum *m_pEnumObj;
	std::vector<CString> *m_pEnumData;
	CString m_Result;
	int m_Choosen;

// Dialog Data
	//{{AFX_DATA(CChooseFromListDlg)
	enum { IDD = IDD_CHOOSEFROMLIST };
	CListBox	m_EnumList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseFromListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseFromListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblclkEnumlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEFROMLISTDLG_H__73436BF1_272B_4800_9B03_501709644D43__INCLUDED_)
