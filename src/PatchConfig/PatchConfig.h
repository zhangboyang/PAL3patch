// PatchConfig.h : main header file for the PATCHCONFIG application
//

#if !defined(AFX_PATCHCONFIG_H__A34D8E26_5B6E_4DBC_908D_D223A096D326__INCLUDED_)
#define AFX_PATCHCONFIG_H__A34D8E26_5B6E_4DBC_908D_D223A096D326__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigApp:
// See PatchConfig.cpp for the implementation of this class
//

class CPatchConfigApp : public CWinApp
{
public:
	CPatchConfigApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatchConfigApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPatchConfigApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern void DoEvents();

extern void *Malloc(size_t n);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATCHCONFIG_H__A34D8E26_5B6E_4DBC_908D_D223A096D326__INCLUDED_)
