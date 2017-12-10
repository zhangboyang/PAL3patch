#ifndef PAL3APATCH_FRAMEWORK_H
#define PAL3APATCH_FRAMEWORK_H
// PATCHAPI DEFINITIONS

extern PATCHAPI void memcpy_to_process(unsigned dest, const void *src, unsigned size);
extern PATCHAPI void memcpy_from_process(void *dest, unsigned src, unsigned size);
extern PATCHAPI void make_branch(unsigned addr, unsigned char opcode, const void *jtarget, unsigned size);
extern PATCHAPI unsigned get_branch_jtarget(unsigned addr, unsigned char opcode);
extern PATCHAPI void make_jmp(unsigned addr, const void *jtarget);
extern PATCHAPI void make_call(unsigned addr, const void *jtarget);
extern PATCHAPI void make_wrapper_branch(unsigned addr, const void *jtarget);
extern PATCHAPI void make_wrapper_branch_batch(unsigned *addr_list, int count, const void *jtarget);
extern PATCHAPI void make_uint(unsigned addr, unsigned uint);
extern PATCHAPI void make_pointer(unsigned addr, void *ptr);
extern PATCHAPI void check_code(unsigned addr, const void *code, unsigned size);
extern PATCHAPI unsigned get_module_base(const char *modulename);
extern PATCHAPI void *get_func_address(const char *dllname, const char *funcname);
extern PATCHAPI void hook_iat(void *iatbase, void *oldptr, void *newptr);
extern PATCHAPI void *hook_import_table(void *image_base, const char *dllname, const char *funcname, void *newptr);
extern PATCHAPI void *alloc_dyncode_buffer(unsigned size);
extern PATCHAPI void add_dyncode_with_jmpback(unsigned patchaddr, unsigned jmpback, void *code, unsigned size);
extern PATCHAPI void flush_instruction_cache(void *base, unsigned size);
extern PATCHAPI void *dup_vftable(void *vftable, unsigned size);

#define SIMPLE_PATCH(addr, oldcode, newcode, size) \
    do { \
        check_code(addr, oldcode, size); \
        memcpy_to_process(addr, newcode, size); \
    } while (0)
#define SIMPLE_PATCH_NOP(addr, oldcode, size) \
    do { \
        unsigned char __nop[size]; \
        memset(__nop, NOP, size); \
        SIMPLE_PATCH(addr, oldcode, __nop, size); \
    } while (0)



#ifndef NO_VARIADIC_MACROS

#define INIT_WRAPPER_CALL(wrapper_func, ...) \
    do { \
        unsigned __caller_list[] = __VA_ARGS__; \
        make_wrapper_branch_batch(__caller_list, sizeof(__caller_list) / sizeof(__caller_list[0]), (wrapper_func)); \
    } while (0)
#define INIT_WRAPPER_VFPTR(wrapper_func, vfptr_addr) \
    do { \
        make_pointer((vfptr_addr), (wrapper_func)); \
    } while (0)
#define PATCH_FLOAT_MEMREF_PTR(float_addr, ...) \
    do { \
        float *__new_value_addr = (float_addr); \
        unsigned __instr_addr[] = __VA_ARGS__; \
        int __instr_addr_cnt = sizeof(__instr_addr) / sizeof(__instr_addr[0]); \
        unsigned *__instr_addr_ptr; \
        for (__instr_addr_ptr = __instr_addr; __instr_addr_ptr < __instr_addr + __instr_addr_cnt; __instr_addr_ptr++) { \
            memcpy_to_process(*__instr_addr_ptr + 2, &__new_value_addr, sizeof(__new_value_addr)); \
        } \
    } while (0)
#define PATCH_FLOAT_MEMREF_EXPR(expr, ...) \
    do { \
        static float __expr_value; \
        __expr_value = (expr); \
        PATCH_FLOAT_MEMREF_PTR(&__expr_value, __VA_ARGS__); \
    } while (0)

#endif // NO_VARIADIC_MACROS



#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#endif
#endif
