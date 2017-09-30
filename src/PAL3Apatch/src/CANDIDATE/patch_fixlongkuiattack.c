#include "common.h"

MAKE_PATCHSET(fixlongkuiattack)
{
    make_uint(0x0056FFB8, 0x3F800000); // float 1.0
}
