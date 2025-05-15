#include "common.h"


static UINT hook_codepage(UINT old_codepage)
{
    return old_codepage == CP_ACP || old_codepage == CP_THREAD_ACP || old_codepage == system_codepage || old_codepage == CODEPAGE_CHS || old_codepage == CODEPAGE_CHT ? target_codepage : old_codepage;
}

static int WINAPI My_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    CodePage = hook_codepage(CodePage);
    return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

static int WINAPI My_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    CodePage = hook_codepage(CodePage);
    return WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

static void check_iconv()
{
    static const char helloworld_chs[] = "\xC4\xE3\xBA\xC3\xCA\xC0\xBD\xE7";
    static const char helloworld_cht[] = "\xA7\x41\xA6\x6E\xA5\x40\xAC\xC9";
    static const wchar_t helloworld_wstr[] = { 0x4F60, 0x597D, 0x4E16, 0x754C, 0 };
    const char *helloworld_cstr;
    char buf[MAXLINE];
    wchar_t wbuf[MAXLINE];
    
    switch (target_codepage) {
        case CODEPAGE_CHS: helloworld_cstr = helloworld_chs; break;
        case CODEPAGE_CHT: helloworld_cstr = helloworld_cht; break;
        default: return;
    }
    
    memset(wbuf, 0, sizeof(wbuf));
    if (MultiByteToWideChar(target_codepage, 0, helloworld_cstr, -1, wbuf, MAXLINE) != (int) wcslen(helloworld_wstr) + 1) goto fail;
    if (wcscmp(wbuf, helloworld_wstr) != 0) goto fail;
    
    memset(buf, 0, sizeof(buf));
    if (WideCharToMultiByte(target_codepage, 0, helloworld_wstr, -1, buf, MAXLINE, NULL, NULL) != (int) strlen(helloworld_cstr) + 1) goto fail;
    if (strcmp(buf, helloworld_cstr) != 0) goto fail;
    
    return;
fail:
    if (MessageBoxW(NULL, wstr_badiconv_text, wstr_badiconv_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) != IDYES) {
        die(0);
    }
}

MAKE_PATCHSET(setlocale)
{   
    if (flag == GAME_LOCALE_AUTODETECT) { // auto detect
        flag = game_locale;
        
        if (flag == GAME_LOCALE_UNKNOWN) {
            warning("unable to detect game locale, fallback to system default.");
            return;
        }
    }
    
    if (flag == GAME_LOCALE_CHS) {
        target_codepage = CODEPAGE_CHS;
        game_locale = GAME_LOCALE_CHS;
    } else if (flag == GAME_LOCALE_CHT) {
        target_codepage = CODEPAGE_CHT;
        game_locale = GAME_LOCALE_CHT;
    } else {
        fail("unknown language flag %d in setlocale.", flag);
    }
    
    if (system_codepage != target_codepage) {
        
        if (GET_PATCHSET_FLAG(testcombat) || GET_PATCHSET_FLAG(uireplacefont) != 1) {
            check_iconv();
            
            // hook GBENGINE.DLL's IAT
            make_pointer(gboffset + 0x100F50A0, My_MultiByteToWideChar);
            make_pointer(gboffset + 0x100F50DC, My_WideCharToMultiByte);
            // hook PAL3.EXE's IAT
            make_pointer(0x0056A128, My_MultiByteToWideChar);
            make_pointer(0x0056A12C, My_WideCharToMultiByte);
        }
        
        if (GET_PATCHSET_FLAG(testcombat)) {
            // patch IAT by replacing known function address (address returned by GetProcAddress())
            // note: this method is not compatible with KernelEx for win9x (also, win9x doesn't support unicode)

            if (is_win9x()) {
                warning("setlocale with testcombat doesn't support win9x.");
                return;
            }
            // hook COMCTL32.DLL's IAT for testcombat
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        }
        
        // no need (and shouldn't) to hook myself!
    }
}
