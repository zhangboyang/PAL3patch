// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma warning(disable: 4786)
#define _CRT_SECURE_NO_WARNINGS
#if _MSC_VER >= 1800
#define _WIN32_WINNT 0x0501
#else
#define _WIN32_WINNT 0x0400
#endif

#if !(defined(BUILD_FOR_PAL3) ^ defined(BUILD_FOR_PAL3A))
#error
#endif

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


#include <wininet.h>
#include <wincrypt.h>

#include <vector>
#include <deque>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

#if _MSC_VER < 1800
typedef unsigned int uint32_t;
#else
#include <cstdint>
#endif

#include "Resource.h"
#include "ConfigDescData.h"
#include "PatchConfigDlg.h"

#include "CheckForUpdates.h"
#include "ChooseFromListDlg.h"
#include "ConfigData.h"
#include "D3DEnum.h"
#include "FontEnum.h"
#include "HASH_SHA1.h"
#include "MouseMsgButton.h"
#include "OpenConfigTool.h"
#include "OpenGameFolder.h"
#include "PatchConfig.h"
#include "PatchVersionInfo.h"
#include "PleaseWaitDlg.h"
#include "UACVirtualization.h"
#include "wstr.h"

#define stricmp _stricmp

#define MAXLINE 4096

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

// load CString from StringTable
#define STRTABLE(x) (CString(MAKEINTRESOURCE((x))))
#define EMPTYSTR (CString(_T("")))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0805558B_5300_41CA_A709_53787B08FC89__INCLUDED_)
