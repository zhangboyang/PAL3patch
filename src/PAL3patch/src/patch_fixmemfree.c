#include "common.h"

MAKE_PATCHSET(fixmemfree)
{
    // redirect two buggy operator_delete() to GBENGINE.DLL's free()
    make_call(0x004B6FDE, gb_mem_allocator.free);
    make_call(0x0053D41E, gb_mem_allocator.free);
}
