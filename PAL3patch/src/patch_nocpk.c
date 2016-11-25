#include "common.h"

MAKE_PATCHSET(nocpk)
{
    config_flags &= ~2;
    SIMPLE_PATCH(0x004047D1, "\x0C\x03", "\x0C\x01", 2);
}
