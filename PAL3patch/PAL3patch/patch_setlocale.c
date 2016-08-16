#include <windows.h>
#include "common.h"

static UINT system_codepage;
static UINT target_codepage;

static UINT hook_codepage(UINT old_codepage)
{
    return old_codepage == CP_ACP || old_codepage == CP_THREAD_ACP || old_codepage == system_codepage || old_codepage == 936 || old_codepage == 950 ? target_codepage : old_codepage;
}

static int WINAPI (*Real_MultiByteToWideChar)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
static int WINAPI My_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    CodePage = hook_codepage(CodePage);
    return Real_MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

static int WINAPI (*Real_WideCharToMultiByte)(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
static int WINAPI My_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    CodePage = hook_codepage(CodePage);
    return Real_WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}


MAKE_PATCHSET(setlocale)
{
    system_codepage = GetACP();
    
    if (flag == 1) target_codepage = 936;
    else if (flag == 2) target_codepage = 950;
    else fail("unknown language flag %d in setlocale.", flag);
    
    if (system_codepage != target_codepage) {
        // patch IAT by replacing known function address (address returned by GetProcAddress())
        // note: this method is not compatible with KernelEx for win9x
        
        // hook GBENGINE.DLL's IAT
        Real_MultiByteToWideChar = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
        Real_WideCharToMultiByte = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        // hook PAL3.EXE's IAT, note we must hardcode IAT address since PAL3.EXE is packed
        hook_iat(TOPTR(0x56A024), Real_MultiByteToWideChar, My_MultiByteToWideChar);
        hook_iat(TOPTR(0x56A024), Real_WideCharToMultiByte, My_WideCharToMultiByte);
        
        if (GET_PATCHSET_FLAG(testcombat)) {
            // hook COMCTL32.DLL's IAT for testcombat
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        }
    }
}
