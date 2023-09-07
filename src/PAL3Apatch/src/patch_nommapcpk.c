#include "common.h"

static VOID WINAPI GetSystemInfo_wrapper(LPSYSTEM_INFO lpSystemInfo)
{
    GetSystemInfo(lpSystemInfo);
    lpSystemInfo->dwAllocationGranularity = 1;
}
static HANDLE WINAPI CreateFileMappingA_wrapper(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName)
{
    return hFile;
}
static BOOL WINAPI CloseHandle_wrapper(HANDLE hObject)
{
    return TRUE;
}
static LPVOID WINAPI MapViewOfFile_wrapper(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
{
    void *p = NULL;
    DWORD nBytesRead;
    
    if (dwFileOffsetHigh != 0) goto fail;
    
    p = malloc(dwNumberOfBytesToMap);
    if (!p) goto fail;
    
    if (SetFilePointer(hFileMappingObject, dwFileOffsetLow, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) goto fail;
    
    if (!ReadFile(hFileMappingObject, p, dwNumberOfBytesToMap, &nBytesRead, NULL) || dwNumberOfBytesToMap != nBytesRead) goto fail;
    
    return p;
fail:
    free(p);
    return NULL;
}
static BOOL WINAPI UnmapViewOfFile_wrapper(LPCVOID lpBaseAddress)
{
    free((void *) lpBaseAddress);
    return TRUE;
}

MAKE_PATCHSET(nommapcpk)
{
    if (flag == 2 || is_win9x()) {
        make_call6(gboffset + 0x1002B26A, GetSystemInfo_wrapper);
        make_call6(gboffset + 0x1002B2E6, CreateFileMappingA_wrapper);
        make_call6(gboffset + 0x1002B304, CloseHandle_wrapper);
        make_call6(gboffset + 0x1002B332, MapViewOfFile_wrapper);
        make_call6(gboffset + 0x1002B354, UnmapViewOfFile_wrapper);
    }
}
