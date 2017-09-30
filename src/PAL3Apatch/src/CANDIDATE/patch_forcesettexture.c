#include "common.h"

MAKE_PATCHSET(forcesettexture)
{
    SIMPLE_PATCH_NOP(gboffset + 0x1001B43D, "\x74\x23", 2);
}
