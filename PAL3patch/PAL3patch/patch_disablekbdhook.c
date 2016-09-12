#include "common.h"

MAKE_PATCHSET(disablekbdhook)
{
    SIMPLE_PATCH_NOP(0x541964, "\x53\x50\x68\xC0\x17\x54\x00\x6A\x0D", 9);
    SIMPLE_PATCH_NOP(0x541972, "\xFF\x15\x58\xA1\x56\x00", 6);
    SIMPLE_PATCH_NOP(0x541C04, "\x52\xFF\x15\x64\xA1\x56\x00", 7);
}
