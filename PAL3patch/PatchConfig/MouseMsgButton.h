#if !defined(AFX_MOUSEMSGBUTTON_H__BF02358F_A9E9_409A_8248_EA58F50F4604__INCLUDED_)
#define AFX_MOUSEMSGBUTTON_H__BF02358F_A9E9_409A_8248_EA58F50F4604__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MouseMsgButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMouseMsgButton window

class CMouseMsgButton : public CButton
{
// Construction
public:
	CMouseMsgButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMouseMsgButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMouseMsgButton();

	int m_BtnID;
	void *m_CallbackParameter;
	void (*m_OnMouseMoveCallback)(void *, int); // callback(parameter, btnid)

	// Generated message map functions
protected:

	//{{AFX_MSG(CMouseMsgButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOUSEMSGBUTTON_H__BF02358F_A9E9_409A_8248_EA58F50F4604__INCLUDED_)
