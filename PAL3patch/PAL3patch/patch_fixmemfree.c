#include "common.h"

MAKE_PATCHSET(fixmemfree)
{
    // redirect two buggy operator_delete() to GBENGINE.DLL's free()
    make_call(0x004B6FDE, TOPTR(gboffset + 0x100E4B99));
    make_call(0x0053D41E, TOPTR(gboffset + 0x100E4B99));
}
