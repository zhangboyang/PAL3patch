#include "common.h"

static DWORD timeOffset;
static DWORD WINAPI My_timeGetTime(VOID)
{
    return timeGetTime() - timeOffset;
}

static int usePerformanceCounter;

static LARGE_INTEGER countFrequency;
static BOOL WINAPI My_QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    *lpFrequency = countFrequency;
    return TRUE;
}

static LARGE_INTEGER countOffset;
static BOOL WINAPI My_QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    if (!usePerformanceCounter) {
        lpPerformanceCount->QuadPart = My_timeGetTime();
        return TRUE;
    }
    BOOL ret = QueryPerformanceCounter(lpPerformanceCount);
    if (ret) {
        lpPerformanceCount->QuadPart -= countOffset.QuadPart;
    }
    return ret;
}

MAKE_PATCHSET(relativetimer)
{
    timeOffset = timeGetTime();
    
    if (QueryPerformanceFrequency(&countFrequency) && QueryPerformanceCounter(&countOffset)) {
        usePerformanceCounter = 1;
    } else {
        usePerformanceCounter = 0;
        countFrequency.QuadPart = 1000;
    }
    
    // hook timeGetTime
    make_pointer(0x0056A1C0, My_timeGetTime);
    make_pointer(gboffset + 0x100F5228, My_timeGetTime);
    
    // hook QueryPerformanceCounter
    make_pointer(gboffset + 0x100F5080, My_QueryPerformanceFrequency);
    make_pointer(gboffset + 0x100F507C, My_QueryPerformanceCounter);
    
    double highFactor = 4294967296.0;
    memcpy_to_process(gboffset + 0x100F5B30, &highFactor, sizeof(highFactor));
}
