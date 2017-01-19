#include "common.h"

const char patch_version[] = "v1.0 beta";
const char build_date[] = __DATE__ ", " __TIME__;
const char build_compiler[] =
#if defined(__GNUC__) && defined(__VERSION__)
        "gcc " __VERSION__
#else
        "unknown C compiler"
#endif
;

const char build_info[] =
    "  built on: " __DATE__ ", " __TIME__ "\n"
    "  compiler: "
#if defined(__GNUC__) && defined(__VERSION__)
        "gcc " __VERSION__
#else
        "unknown C compiler"
#endif
    "\n";

void show_about()
{
    int flag = get_int_from_configfile("showabout");
    if (flag) {
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), 
            wstr_about_text
            , patch_version, build_info); 
        MessageBoxW(NULL, buf, wstr_about_title, MB_ICONINFORMATION | MB_SETFOREGROUND); 
    }
}
