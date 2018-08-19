#ifndef PAL3PATCH_COMMON_H
#define PAL3PATCH_COMMON_H

// NOTE:
//
//    FOR ALL C SOURCE FILES IN PAL3PATCH ITSELF:
//        SHOULD INCLUDE AND ONLY INCLUDE THIS FILE
//
//    FOR APPLICATIONS USES PAL3PATCH API:
//        SHOULD NOT INCLUDE THIS FILE DIRECTLY
//        SHOULD INCLUDE "PAL3patch.h" INSTEAD
//


// patch version
#define PATCH_VERSION          10700
#define PATCH_VERSION_STRING "v1.7"




// blocks surrounded with "#ifdef PATCHAPI_EXPORTS" is internal definitions
// blocks surrounded with "#ifdef PATCHAPI_IMPORTS" is external definitions
#if !defined(PATCHAPI_IMPORTS) && !defined(PATCHAPI_EXPORTS)
#define PATCHAPI_EXPORTS
#endif
#if defined(PATCHAPI_IMPORTS) + defined(PATCHAPI_EXPORTS) != 1
#error
#endif 
#if defined(__cplusplus) && !defined(PATCHAPI_IMPORTS)
#error
#endif


// dllexport / dllimport
//   can be used with cdecl functions only
//   because our dlltool doesn't support --add-stdcall-underscore
#ifdef PATCHAPI_EXPORTS
#define PATCHAPI __declspec(dllexport)
#else
#define PATCHAPI __declspec(dllimport)
#endif


// test directx sdk
#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define __func__ __FUNCTION__
#define HAVE_D3D9SDK
#endif


// test thiscall feature
#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#define HAVE_THISCALL
#endif

#ifndef NO_VARIADIC_MACROS
#ifdef HAVE_THISCALL
#define MAKE_THISCALL(ret_type, func_name, this_decl, ...) ret_type (__thiscall func_name)(this_decl, ##__VA_ARGS__)
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type (__thiscall *)(this_type, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, ##__VA_ARGS__)
#else
// use fastcall to simulate thiscall
#ifdef __TINYC__
#define MAKE_THISCALL(ret_type, func_name, this_decl, ...) ret_type __fastcall (func_name)(this_decl, int dummy, ##__VA_ARGS__)
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type __fastcall (*)(this_type, int, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, 0, ##__VA_ARGS__)
#else
#define MAKE_THISCALL(ret_type, func_name, this_decl, ...) ret_type (__fastcall func_name)(this_decl, int dummy, ##__VA_ARGS__)
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type (__fastcall *)(this_type, int, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, 0, ##__VA_ARGS__)
#endif
#endif
#endif


// noreturn and inline
#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#define INLINE __inline
#elif defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#define INLINE inline
#else
#define NORETURN
#define INLINE
#endif












#ifdef PATCHAPI_EXPORTS

// internal macros
#define USE_PAL3_DEFINITIONS


// linker configuration
#if defined(_MSC_VER)
#if _MSC_VER < 1800
#error require MSVC++ 2013 or higher
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#if _MSC_VER >= 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#endif



// return address
#if defined(_MSC_VER)
#define func_return_address() _ReturnAddress()
#elif defined(__GNUC__)
#define func_return_address() __builtin_return_address(0)
#else
#error
#endif


#endif // PATCHAPI_EXPORTS












// common macros
#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)
#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)
#define CONCAT3(a, b, c) CONCAT(CONCAT(a, b), c)
#define CONCAT4(a, b, c, d) CONCAT(CONCAT3(a, b, c), d)
#define CONCAT5(a, b, c, d, e) CONCAT(CONCAT4(a, b, c, d), e)
#define CONCAT6(a, b, c, d, e, f) CONCAT(CONCAT5(a, b, c, d, e), f)
#define _WSTR(x) L##x
#define WSTR(x) _WSTR(x)

// type macros
#define TOPTR(addr) ((void *)(addr))
#define TOUINT(x) ((unsigned)(x))
#define TOUCHAR(x) ((unsigned char)(x))

// pointer macros
#define PTRADD(ptr, add) ((void *)(((char *)(ptr)) + (add)))
#define PTRSUB(a, b) (((char *)(a)) - ((char *)(b)))

// round macros
#define ROUND_UP(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define ROUND_DOWN(x, y) ((x) - ((x) % (y)))

// common constants
#define NOP 0x90
#define INT3 0xCC

#define MAXLINE      4096
#define MAXLINEFMT "%4095s"

#define eps (1e-5)
#define inf (1e100)

#define UTF8_BOM_STR "\xEF\xBB\xBF"
#define UTF8_BOM_LEN 3

#define SPACECHAR_LIST " \t\n\v\f\r"










#ifdef PATCHAPI_EXPORTS

#if defined(__GNUC__)
// avoid libmoldname.a
#define _NO_OLDNAMES
#endif

// for InitCommonControlsEx
#define _WIN32_IE	0x0300
// for SHA1
#define _WIN32_WINNT 0x0400


// C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <io.h>

// windows headers
#include <windows.h>
#include <wincrypt.h>
#include <commctrl.h>

// direct3d headers
#include "tiny_d3d9sdk.h"

// patch oldnames
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strdup _strdup
#define wcsdup _wcsdup
#define wcsicmp _wcsicmp

// make sure '\0' is added when using snprintf family functions
#define safe_snprintf_helper(func, s, n, format, ...) ((n) > 0 ? ((s)[(n) - 1] = 0, func((s), (n) - 1, format, ##__VA_ARGS__)) : -1)
#define snprintf(s, n, format, ...) safe_snprintf_helper(_snprintf, s, n, format, ##__VA_ARGS__)
#define snwprintf(s, n, format, ...) safe_snprintf_helper(_snwprintf, s, n, format, ##__VA_ARGS__)
#define vsnprintf(s, n, format, ...) safe_snprintf_helper(_vsnprintf, s, n, format, ##__VA_ARGS__)
#define vsnwprintf(s, n, format, ...) safe_snprintf_helper(_vsnwprintf, s, n, format, ##__VA_ARGS__)


#endif // PATCHAPI_EXPORTS


#ifdef PATCHAPI_IMPORTS

#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

#include <windows.h>
#include "tiny_d3d9sdk.h"

#endif


#if defined(USE_PAL3_DEFINITIONS) && !defined(__cplusplus)
// bool
typedef unsigned char bool;
#define true 1
#define false 0
#endif








#ifdef __cplusplus
extern "C" {
#endif

// PAL3patch headers
#include "memallocator.h"
#include "pal3.h"
#include "hook.h"
#include "asmpatch.h"
#include "about.h"
#include "misc.h"
#include "logger.h"
#include "wstr.h"
#include "cfgreader.h"
#include "locale.h"
#include "framework.h"
#include "transform.h"
#include "patch_common.h"
#include "PAL3patch.h"
#include "cjktable.h"
#include "effecthook.h"
#include "texturehook.h"
#include "ftfont.h"
#include "ftcharhack.h"
#include "plugin.h"
#include "fsutil.h"
#include "bytevector.h"


#ifdef __cplusplus
}
#endif

#endif
