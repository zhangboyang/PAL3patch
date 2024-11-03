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
#define _WIN32_IE 0x0500
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


#include <shlwapi.h>
#include <wininet.h>
#include <wincrypt.h>

#include <io.h>
#include <errno.h>

#include <cassert>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <string>

#if _MSC_VER < 1800
typedef unsigned int uint32_t;
#else
#include <cstdint>
#endif

#include "Resource.h"
#include "ConfigDescData.h"
#include "PatchConfigDlg.h"

#include "BadFiles.h"
#include "CheckForUpdates.h"
#include "ChooseFromListDlg.h"
#include "ConfigData.h"
#include "D3DEnum.h"
#include "FontEnum.h"
#include "HASH_SHA1.h"
#include "MouseMsgButton.h"
#include "OpenConfigTool.h"
#include "OpenGameFolder.h"
#include "ReferenceCounter.h"
#include "ReadWriter.h"
#include "ProgressObject.h"
#include "RepairGameData.h"
#include "XorRepair.h"
#include "BufferReader.h"
#include "CPKFixer.h"
#include "FileFixer.h"
#include "PatchConfig.h"
#include "PatchVersionInfo.h"
#include "PleaseWaitDlg.h"
#include "UACVirtualization.h"
#include "wstr.h"
#include "fsutil.h"
#include "wal.h"

#define strdup _strdup
#define wcsdup _wcsdup

#define MAXLINE 4096

#define PTRADD(ptr, add) ((void *)(((char *)(ptr)) + (add)))

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

// load CString from StringTable
#define STRTABLE(x) (CString(MAKEINTRESOURCE((x))))
#define EMPTYSTR (CString(_T("")))

#define die(status) do { TerminateProcess(GetCurrentProcess(), (status)); while (1); } while (0)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0805558B_5300_41CA_A709_53787B08FC89__INCLUDED_)
