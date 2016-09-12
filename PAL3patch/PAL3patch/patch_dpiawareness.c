#include "common.h"


MAKE_PATCHSET(dpiawareness)
{
    HMODULE hShcore = NULL, hUser32 = NULL;
    hShcore = LoadLibrary("SHCORE.DLL");
    if (hShcore) {
        HRESULT WINAPI (*SetProcessDpiAwareness)(DWORD value) = (void *) GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (SetProcessDpiAwareness) {
            if (SetProcessDpiAwareness(flag) == S_OK) {
                goto done;
            }
        }
    }
    hUser32 = LoadLibrary("USER32.DLL");
    if (hUser32) {
        BOOL WINAPI (*SetProcessDPIAware)(void) = (void *) GetProcAddress(hUser32, "SetProcessDPIAware");
        if (SetProcessDPIAware) {
            SetProcessDPIAware();
            goto done;
        }
    }
done:
    if (hShcore) FreeLibrary(hShcore);
    if (hUser32) FreeLibrary(hUser32);
}
