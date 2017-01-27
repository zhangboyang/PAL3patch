#include "common.h"

struct memory_allocator gb_mem_allocator;
struct memory_allocator def_mem_allocator;

void init_memory_allocators()
{
    gb_mem_allocator = make_memory_allocator(_gbmalloc, _gbfree);
    def_mem_allocator = make_memory_allocator(malloc, free);
}
