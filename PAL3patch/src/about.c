#include "common.h"

const char patch_version[] = "v1.0 beta";
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
#ifdef HAVE_D3DX9
" +d3dx9hdr"
#else
" -d3dx9hdr"
#endif
#ifdef DYNLINK_D3DX9_AT_RUNTIME
" +d3dx9dll"
#else
" -d3dx9dll"
#endif
    "\n";

void show_about()
{
    int flag = get_int_from_configfile("showabout");
    if (flag) {
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), wstr_about_text, patch_version, build_info);
        MessageBoxW(NULL, buf, wstr_about_title, MB_ICONINFORMATION | MB_SETFOREGROUND); 
    }
}
