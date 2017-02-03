#include "common.h"

static DWORD timeOffset;
static DWORD (WINAPI *Real_timeGetTime)(VOID);
static DWORD WINAPI My_timeGetTime(VOID)
{
    return Real_timeGetTime() - timeOffset;
}

static LARGE_INTEGER countOffset;
static BOOL (WINAPI *Real_QueryPerformanceCounter)(LARGE_INTEGER *lpPerformanceCount);
static BOOL WINAPI My_QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    BOOL ret = Real_QueryPerformanceCounter(lpPerformanceCount);
    if (ret) {
        lpPerformanceCount->QuadPart -= countOffset.QuadPart;
    }
    return ret;
}

MAKE_PATCHSET(relativetimer)
{
    // set offsets
    timeOffset = timeGetTime();
    if (!QueryPerformanceCounter(&countOffset)) {
        countOffset.QuadPart = 0;
    }
    
    // hook timeGetTime
    Real_timeGetTime = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "WINMM.DLL", "timeGetTime", My_timeGetTime);
    hook_iat(PAL3_WINMM_IATBASE, Real_timeGetTime, My_timeGetTime);
    
    // hook QueryPerformanceCounter
    Real_QueryPerformanceCounter = hook_import_table(GetModuleHandle("GBENGINE.DLL"), "KERNEL32.DLL", "QueryPerformanceCounter", My_QueryPerformanceCounter);
}
