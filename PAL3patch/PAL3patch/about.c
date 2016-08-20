#include <stdio.h>
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
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), 
            L"欢迎使用《仙剑奇侠传三》增强补丁 %hs\n" 
            L"\n"
            L"本补丁可以修复一些游戏程序的小问题\n"
            L"并且添加了一些实用的小功能\n" 
            L"详细信息和使用方法请参见 PAL3patch_README.txt\n" 
            L"\n"
            L"\n"
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
            , PAL3PATCH_VERSION, build_info, CONFIG_FILE, flag); 
            
            
        MessageBoxW(NULL, buf, L"关于", MB_ICONINFORMATION | MB_SETFOREGROUND); 
    }
}

