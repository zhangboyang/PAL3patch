#ifndef PAL3PATCH_MEMALLOCATOR_H
#define PAL3PATCH_MEMALLOCATOR_H

extern void init_memory_allocators();

// memory allocator structure
typedef void *(*malloc_funcptr_t)(size_t);
typedef void (*free_funcptr_t)(void *);
struct memory_allocator {
    malloc_funcptr_t malloc;
    free_funcptr_t free;
};
#define make_memory_allocator(_malloc, _free) \
    ((struct memory_allocator) { \
        .malloc = (_malloc), \
        .free = (_free), \
    })

// allocators for different modules
extern struct memory_allocator gb_mem_allocator; // GBENGINE.DLL
extern struct memory_allocator def_mem_allocator; // PAL3patch's default allocator

#endif
