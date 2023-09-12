#include "common.h"

static HMODULE lazyShcore(void)
{
    static HMODULE hShcore = NULL;
    if (!hShcore) hShcore = LoadLibrary("SHCORE.DLL");
    return hShcore;
}

static HMODULE lazyUser32(void)
{
    static HMODULE hUser32 = NULL;
    if (!hUser32) hUser32 = LoadLibrary("USER32.DLL");
    return hUser32;
}

#define myDPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((void *)-3)
#define myDPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((void *)-4)
static int trySetProcessDpiAwarenessContext(void *value)
{
    HMODULE hUser32 = lazyUser32();
    if (hUser32) {
        BOOL (WINAPI *mySetProcessDpiAwarenessContext)(void *) = (void *) GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (mySetProcessDpiAwarenessContext) {
            if (mySetProcessDpiAwarenessContext(value)) {
                return 1;
            }
        }
    }
    return 0;
}

#define myPROCESS_PER_MONITOR_DPI_AWARE 2
static int trySetProcessDpiAwareness(int value)
{
    HMODULE hShcore = lazyShcore();
    if (hShcore) {
        HRESULT (WINAPI *mySetProcessDpiAwareness)(int) = (void *) GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (mySetProcessDpiAwareness) {
            if (mySetProcessDpiAwareness(value) == S_OK) {
                return 1;
            }
        }
    }
    return 0;
}

static int trySetProcessDPIAware(void)
{
    HMODULE hUser32 = lazyUser32();
    if (hUser32) {
        BOOL (WINAPI *mySetProcessDPIAware)(void) = (void *) GetProcAddress(hUser32, "SetProcessDPIAware");
        if (mySetProcessDPIAware) {
            if (mySetProcessDPIAware()) {
                return 1;
            }
        }
    }
    return 0;
}

MAKE_PATCHSET(dpiawareness)
{
    switch (flag) {
    case 3:
        if (trySetProcessDpiAwarenessContext(myDPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) return;
    case 2:
        if (trySetProcessDpiAwarenessContext(myDPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) return;
        if (trySetProcessDpiAwareness(myPROCESS_PER_MONITOR_DPI_AWARE)) return;
    case 1:
        if (trySetProcessDPIAware()) return;
    }
}
