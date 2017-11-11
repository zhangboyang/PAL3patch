#include "common.h"

MAKE_PATCHSET(fixjusticebookui)
{
    // patch 1024x768 check
    SIMPLE_PATCH(0x0047D526, "\x83\xE0\x23", "\x31\xC0\x90", 3);
    SIMPLE_PATCH(0x0047D5EF, "\x83\xE0\x28", "\x31\xC0\x90", 3);
}
