#include "common.h"

static DWORD timeOffset;
static DWORD WINAPI My_timeGetTime(VOID)
{
    return timeGetTime() - timeOffset;
}

static LARGE_INTEGER countOffset;
static BOOL WINAPI My_QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    BOOL ret = QueryPerformanceCounter(lpPerformanceCount);
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
    make_pointer(0x005581C8, My_timeGetTime);
    make_pointer(gboffset + 0x100D61F4, My_timeGetTime);
    
    // hook QueryPerformanceCounter
    make_pointer(gboffset + 0x100D6080, My_QueryPerformanceCounter);
}
