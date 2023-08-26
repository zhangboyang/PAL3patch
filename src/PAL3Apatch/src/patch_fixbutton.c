#include "common.h"

MAKE_PATCHSET(fixbutton)
{
    SIMPLE_PATCH(0x00443392, "\x74\x0C", "\xEB\x0C", 2); // FIXME: side effects?
}
