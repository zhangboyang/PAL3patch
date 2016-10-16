#include "common.h"

// language, see patch_setlocale.c for details
UINT system_codepage;
UINT target_codepage;


static const wchar_t wstr_about_title_CHS[] = L"关于";
static const wchar_t wstr_about_text_CHS[] = 
    L"欢迎使用《仙剑奇侠传三》增强补丁 %hs\n" 
    L"\n"
    L"本补丁可以修复一些游戏程序的小问题\n"
    L"并且添加了一些实用的小功能\n" 
    L"详细信息和使用方法请参见补丁说明文件\n"
    L"\n"
    L"\n"
    L"如果您不想每次启动时看到此信息\n"
    L"请使用补丁配置工具，将“显示关于”设为“禁用”\n" 
    L"\n"
    L"\n"
    L"源代码:\n"
    L"  https://github.com/zhangboyang/PAL3patch\n"
    L"编译信息:\n%hs"
;
static const wchar_t wstr_confirmquit_text_CHS[] = L"您确定要退出游戏吗？\n未保存的游戏进度将会丢失。";
static const wchar_t wstr_confirmquit_title_CHS[] = L"退出";


const wchar_t *wstr_about_title = wstr_about_title_CHS;
const wchar_t *wstr_about_text = wstr_about_text_CHS;
const wchar_t *wstr_confirmquit_text = wstr_confirmquit_text_CHS;
const wchar_t *wstr_confirmquit_title = wstr_confirmquit_title_CHS;


void init_locale()
{
    // no translations yet
}
