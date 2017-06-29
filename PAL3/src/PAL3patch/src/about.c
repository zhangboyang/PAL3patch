#include "common.h"

#if defined(__GNUC__) && defined(__VERSION__)
#define BUILD_COMPILER "gcc " __VERSION__
#elif defined(_MSC_VER)
#define BUILD_COMPILER "Microsoft C " TOSTR(_MSC_VER)
#else
#define BUILD_COMPILER "unknown C compiler"
#endif

const char patch_version[] = "v1.3";
const char build_date[] = __DATE__ ", " __TIME__;
const char build_compiler[] = BUILD_COMPILER;

const char build_info[] =
    "  built on: " __DATE__ ", " __TIME__ "\n"
    "  compiler: " BUILD_COMPILER "\n"
    "  config:"
#ifdef HAVE_THISCALL
" +thiscall"
#else
" -thiscall"
#endif
#ifdef HAVE_D3D9SDK
" +d3d9sdk"
#else
" -d3d9sdk"
#endif
#ifdef DYNLINK_D3DX9_AT_RUNTIME
" +d3dx9dll"
#else
" -d3dx9dll"
#endif
    "\n";

const char lib_info[] =
    "  d3d9: " TOSTR(D3D_SDK_VERSION) "\n"
    "  d3dx9: " TOSTR(D3DX_SDK_VERSION) "\n"
    "  freetype: " FTFONT_VERSTR "\n"
;

void show_about()
{
    int flag = get_int_from_configfile("showabout");
    if (flag) {
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), wstr_about_text, patch_version, build_info);
        MessageBoxW(NULL, buf, wstr_about_title, MB_ICONINFORMATION | MB_SETFOREGROUND); 
    }
}
