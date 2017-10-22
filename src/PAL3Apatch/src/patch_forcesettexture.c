#include "common.h"

MAKE_PATCHSET(forcesettexture)
{
    SIMPLE_PATCH_NOP(gboffset + 0x1001ADC3, "\x74\x24", 2);
}
