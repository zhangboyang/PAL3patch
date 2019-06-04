#include "common.h"

static MAKE_ASMPATCH(cdpatch)
{
    memset(TOPTR(0x1894970), 0, 0x104);
    strcpy(TOPTR(0x1894970), "."); // fake PAL3DataPath value
    M_DWORD(R_EBP - 4) = R_EBX; // oldcode
    // ECX is not perserved
    R_EDI = 0x1894970 + 0x104;
    R_EAX = 1; // fake return value
}


MAKE_PATCHSET(cdpatch)
{
    INIT_ASMPATCH(cdpatch, 0x0054197B, 0x28,
        "\xB9\x41\x00\x00\x00\x33\xC0\xBF\x70\x49\x89\x01\x68\x70\x49\x89"
        "\x01\x68\x28\x90\x60\x00\xF3\xAB\x68\x10\x90\x60\x00\x89\x5D\xFC"
        "\xE8\xA0\x02\x00\x00\x83\xC4\x0C");
}
