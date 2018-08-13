#include "common.h"

const char patch_version[] = PATCH_VERSION_STRING;
const char build_date[] = __DATE__ ", " __TIME__;
const char build_compiler[] = BUILD_COMPILER;

const char build_info[] =
    "  built on: " BUILD_DATE "\n"
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
"\n"
    "  library:"
        " d3d9/" TOSTR(D3D_SDK_VERSION)
        " d3dx9/" TOSTR(D3DX_SDK_VERSION)
        " freetype/" FTFONT_VERSTR
"\n"
    ;

void show_about()
{
    int flag = get_int_from_configfile("showabout");
    if (flag) {
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), wstr_about_text, patch_version, build_info);
        MessageBoxW(NULL, buf, wstr_about_title, MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND); 
    }
}
