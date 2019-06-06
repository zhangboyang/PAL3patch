#include "common.h"

MAKE_PATCHSET(disableime)
{
    HMODULE hImm32 = NULL;
    hImm32 = LoadLibrary("IMM32.DLL");
    if (hImm32) {
        BOOL (WINAPI *myImmDisableIME)(DWORD) = (void *) GetProcAddress(hImm32, "ImmDisableIME");
        if (myImmDisableIME) {
            myImmDisableIME(-1);
        }
        FreeLibrary(hImm32);
    }
}
