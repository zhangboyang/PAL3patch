#ifndef PAL3PATCH_COMMON_H
#define PAL3PATCH_COMMON_H

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

// use fastcall to simulate thiscall
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type __fastcall (*)(this_type, int, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, 0, ##__VA_ARGS__)


// common constants
#define NOP 0x90
#define INT3 0xCC
#define PAGE_SIZE 4096

#define MAXLINE 4096
#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"




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

#include <windows.h>
#include <wincrypt.h>
#include <commctrl.h>
#include <d3d9.h>


// PAL3patch headers
#include "gbengine.h"
#include "zpk.h"
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
#include "d3dx9.h"


// common typedefs
typedef void *(*malloc_funcptr_t)(size_t);
typedef void (*free_funcptr_t)(void *);


#endif
