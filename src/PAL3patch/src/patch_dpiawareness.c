#include "common.h"


MAKE_PATCHSET(dpiawareness)
{
    HMODULE hShcore = NULL, hUser32 = NULL;
    hShcore = LoadLibrary("SHCORE.DLL");
    if (hShcore) {
        HRESULT (WINAPI *mySetProcessDpiAwareness)(DWORD value) = (void *) GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (mySetProcessDpiAwareness) {
            if (mySetProcessDpiAwareness(flag) == S_OK) {
                goto done;
            }
        }
    }
    hUser32 = LoadLibrary("USER32.DLL");
    if (hUser32) {
        BOOL (WINAPI *mySetProcessDPIAware)(void) = (void *) GetProcAddress(hUser32, "SetProcessDPIAware");
        if (mySetProcessDPIAware) {
            mySetProcessDPIAware();
            goto done;
        }
    }
done:
    if (hShcore) FreeLibrary(hShcore);
    if (hUser32) FreeLibrary(hUser32);
}
