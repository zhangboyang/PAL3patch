// misc functions
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "common.h"

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
        char cfgbuf[MAXLINE];
        get_all_config(cfgbuf, sizeof(cfgbuf));
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), 
            L"欢迎使用《仙剑奇侠传三》增强补丁\n"
            L"\n"
            L"本补丁可以修复一些游戏程序编写不合理的地方\n"
            L"并且添加了一些实用的小功能\n" 
            L"详细信息和使用方法请参见 PAL3patch_README.txt\n" 
            L"\n"
            L"\n"
            L"配置选项:\n%hs" 
            L"源代码:\n"
            L"  https://github.com/zhangboyang/PAL3patch\n"
            L"编译信息:\n%hs"
            L"\n"
            L"\n"
            L"如果您不想每次启动时看到此信息\n"
            L"请将配置文件 %hs 中的\n"
            L"  showabout=%d\n"
            L"修改为\n"
            L"  showabout=0\n"
            L"\n"
            , cfgbuf, build_info, CONFIG_FILE, flag); 
            
            
        MessageBoxW(NULL, buf, L"关于", MB_ICONINFORMATION); 
    }
}

void __fail(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    int len;
    snprintf(buf, sizeof(buf), "  file: %s\n  line: %d\n  func: %s\nmessage:\n  ", file, line, func);
    len = strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    OutputDebugString(buf);
    FILE *fp = fopen(ERROR_FILE, "w");
    if (fp) {
        fputs("build information:\n", fp);
        fputs(build_info, fp);
        fputs("error details:\n", fp);
        fputs(buf, fp);
        fputc('\n', fp);
        fclose(fp);
    }
    MessageBoxA(NULL, buf + len, NULL, MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 1);
    va_end(ap);
}
