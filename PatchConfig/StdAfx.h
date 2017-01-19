// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// disable C4786
#pragma warning(disable: 4786)
// for CryptoAPI
#define _WIN32_WINNT 0x0400


#if !defined(AFX_STDAFX_H__0805558B_5300_41CA_A709_53787B08FC89__INCLUDED_)
#define AFX_STDAFX_H__0805558B_5300_41CA_A709_53787B08FC89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <vector>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

#include "D3DEnum.h"
#include "ConfigDescData.h"


// load CString from StringTable
#define STRTABLE(x) (CString(MAKEINTRESOURCE((x))))
#define EMPTYSTR (CString(_T("")))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0805558B_5300_41CA_A709_53787B08FC89__INCLUDED_)
