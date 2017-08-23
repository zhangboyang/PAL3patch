#include "common.h"

struct memory_allocator pal3_mem_allocator;
struct memory_allocator gb_mem_allocator;
struct memory_allocator patch_mem_allocator;

void init_memory_allocators()
{
    pal3_mem_allocator = make_memory_allocator(pal3malloc, pal3free);
    gb_mem_allocator = make_memory_allocator(gbmalloc, gbfree);
    patch_mem_allocator = make_memory_allocator(malloc, free);
}
