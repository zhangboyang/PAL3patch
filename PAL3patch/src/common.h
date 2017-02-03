#ifndef PAL3PATCH_COMMON_H
#define PAL3PATCH_COMMON_H

// compiler capability
#if (defined(_MSC_VER) && _MSC_VER >= 1400) || \
    (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))
#define HAVE_THISCALL
#endif

#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#elif
#define NORETURN
#endif

#ifdef HAVE_THISCALL
#define MAKE_THISCALL(ret_type, func_name, this_decl, ...) ret_type __thiscall func_name(this_decl, ##__VA_ARGS__)
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type (__thiscall *)(this_type, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, ##__VA_ARGS__)
#else
// use fastcall to simulate thiscall
#define MAKE_THISCALL(ret_type, func_name, this_decl, ...) ret_type (__fastcall func_name)(this_decl, int dummy, ##__VA_ARGS__)
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type (__fastcall *)(this_type, int, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, 0, ##__VA_ARGS__)
#endif

#if defined(__GNUC__) && defined(__VERSION__)
#define BUILD_COMPILER "gcc " __VERSION__
#elif defined(_MSC_VER)
#define BUILD_COMPILER "Microsoft C " TOSTR(_MSC_VER)
#else
#define BUILD_COMPILER "unknown C compiler"
#endif



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

// pointer macros
#define TOPTR(addr) ((void *)(addr))
#define TOUINT(addr) ((unsigned)(addr))
#define PTRADD(ptr, add) ((void *)(((char *)(ptr)) + (add)))

// common constants
#define NOP 0x90
#define INT3 0xCC
#define PAGE_SIZE 4096


#define MAXLINE      4096
#define MAXLINEFMT "%4095s"

#define eps (1e-5)


// bool
typedef unsigned char bool;
#define true 1
#define false 0

// avoid libmoldname.a
#define _NO_OLDNAMES

// for InitCommonControlsEx
#define _WIN32_IE	0x0300
// for SHA1
#define _WIN32_WINNT 0x0400


// system headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

#include <windows.h>
#include <wincrypt.h>
#include <commctrl.h>
#include <d3d9.h>


// PAL3patch headers
#include "memallocator.h"
#include "pal3.h"
#include "sha1.h"
#include "hook.h"
#include "asmpatch.h"
#include "about.h"
#include "misc.h"
#include "cfgreader.h"
#include "locale.h"
#include "framework.h"
#include "transform.h"
#include "patch_common.h"
#include "PAL3patch.h"
#include "d3dx9_21.h"
#include "cjktable.h"
#include "effecthook.h"
#include "texturehook.h"


// patch oldnames
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strdup _strdup
#define wcsdup _wcsdup
#define snwprintf _snwprintf

#endif
