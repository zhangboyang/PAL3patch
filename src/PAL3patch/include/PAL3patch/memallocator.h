#ifndef PAL3PATCH_MEMALLOCATOR_H
#define PAL3PATCH_MEMALLOCATOR_H
// PATCHAPI DEFINITIONS

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
extern PATCHAPI struct memory_allocator pal3_mem_allocator;  // PAL3.EXE
extern PATCHAPI struct memory_allocator gb_mem_allocator;    // GBENGINE.DLL
extern PATCHAPI struct memory_allocator patch_mem_allocator; // PAL3PATCH.DLL

#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define cur_mem_allocator patch_mem_allocator

extern void init_memory_allocators(void);

#endif
#endif
