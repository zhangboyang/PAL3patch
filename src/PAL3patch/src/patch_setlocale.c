#include "common.h"


static UINT hook_codepage(UINT old_codepage)
{
    return old_codepage == CP_ACP || old_codepage == CP_THREAD_ACP || old_codepage == system_codepage || old_codepage == 936 || old_codepage == 950 ? target_codepage : old_codepage;
}

static int (WINAPI *Real_MultiByteToWideChar)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
static int WINAPI My_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    CodePage = hook_codepage(CodePage);
    return Real_MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

static int (WINAPI *Real_WideCharToMultiByte)(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
static int WINAPI My_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    CodePage = hook_codepage(CodePage);
    return Real_WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

static int detect_locale()
{
    // read compressed data of "basedata.cpk\datascript\lang.txt"
    // and use magic string to determine game locale
    
    const char *CHS_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xCF\xC9\xBD\xA3\xC6\xE6\xCF\xC0\xB4\xAB\x33\x26";
    const char *CHT_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xA5\x50\xBC\x43\xA9\x5F\xAB\x4C\xB6\xC7\x33\x26";
    size_t CHS_magic_len = strlen(CHS_magic);
    size_t CHT_magic_len = strlen(CHT_magic);
    ULONG key_CRC = 0xCB283888; // equals gbCrc32Compute("datascript\\lang.txt"), but we can't call gbCrc32Compute() at this time
    
    int result = 0;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapFile = NULL;
    LPVOID lpMapAddress = NULL;

    unsigned i;
    int left, right, mid;
    ULONG base, skip;
    SYSTEM_INFO SysInfo;
    DWORD dwSysGran;
    struct CPKHeader cpkhdr;
    struct CPKTable *cpktbl;
    struct CPKTable cpkitem;
    void *pdata;
    ULONG datasz;
    
    // create file handle
    hFile = CreateFile("basedata\\basedata.cpk", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) goto done;
    
    // create file mapping handle
    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapFile) goto done;
    
    // get memory allocation granularity
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;
    
    // map CPK header
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(struct CPKHeader));
    if (!lpMapAddress) goto done;
    
    // read CPK header
    memcpy(&cpkhdr, lpMapAddress, sizeof(cpkhdr));
    
    // map CPK index table
    UnmapViewOfFile(lpMapAddress);
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(struct CPKHeader) + sizeof(struct CPKTable) * cpkhdr.dwMaxFileNum);
    if (!lpMapAddress) goto done;
    cpktbl = PTRADD(lpMapAddress, sizeof(struct CPKHeader));
    
    // search CPK table entry
    left = 0;
    right = cpkhdr.dwValidTableNum;
	while (1) {
        if (left == right) goto done;
        mid = left + (right - left) / 2;
        if (cpktbl[mid].dwCRC == key_CRC && (cpktbl[mid].dwFlag & 0x1) && !(cpktbl[mid].dwFlag & 0x10)) {
            break;
        }
        if (left + 1 == right) goto done;
        if (key_CRC >= cpktbl[mid].dwCRC) {
            left = mid;
        } else {
            right = mid;
        }
    }
	memcpy(&cpkitem, &cpktbl[mid], sizeof(cpkitem));
	
	// map file data
	base = ROUND_DOWN(cpkitem.dwStartPos, dwSysGran);
	skip = cpkitem.dwStartPos - base;
    UnmapViewOfFile(lpMapAddress);
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, base, skip + cpkitem.dwPackedSize);
    if (!lpMapAddress) goto done;
    pdata = PTRADD(lpMapAddress, skip);
    datasz = cpkitem.dwPackedSize;
	
	// process data
	for (i = 0; i < datasz; i++) {
        if (i + CHS_magic_len <= datasz && memcmp(PTRADD(pdata, i), CHS_magic, CHS_magic_len) == 0) {
            result = 1;
            break;
        }
        if (i + CHT_magic_len <= datasz && memcmp(PTRADD(pdata, i), CHT_magic, CHT_magic_len) == 0) {
            result = 2;
            break;
        }
    }
	
done:
    if (lpMapAddress) UnmapViewOfFile(lpMapAddress);
    if (hMapFile) CloseHandle(hMapFile);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    
    return result;
}

MAKE_PATCHSET(setlocale)
{   
    if (flag == 3) { // auto detect
        flag = detect_locale();
        
        if (flag == 0) {
            warning("unable to detect game locale, fallback to system default.");
            return;
        }
    }
    
    if (flag == 1) target_codepage = 936; // CHS
    else if (flag == 2) target_codepage = 950; // CHT
    else fail("unknown language flag %d in setlocale.", flag);
    
    if (system_codepage != target_codepage) {
        // patch IAT by replacing known function address (address returned by GetProcAddress())
        // note: this method is not compatible with KernelEx for win9x (also, win9x doesn't support unicode)
        if (is_win9x()) {
            warning("setlocale doesn't support win9x.");
            return;
        }
    
        // hook GBENGINE.DLL's IAT
        Real_MultiByteToWideChar = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
        Real_WideCharToMultiByte = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        // hook PAL3.EXE's IAT
        hook_import_table(GetModuleHandle(NULL), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
        hook_import_table(GetModuleHandle(NULL), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        
        if (GET_PATCHSET_FLAG(testcombat)) {
            // hook COMCTL32.DLL's IAT for testcombat
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "MultiByteToWideChar", My_MultiByteToWideChar);
            hook_import_table(GetModuleHandle("COMCTL32.DLL"), "KERNEL32.DLL", "WideCharToMultiByte", My_WideCharToMultiByte);
        }
        
        // no need (and shouldn't) to hook myself!
    }
}
