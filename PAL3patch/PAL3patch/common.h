#ifndef PAL3PATCH_COMMON_H
#define PAL3PATCH_COMMON_H

// common macros
#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)
#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)
#define CONCAT3(a, b, c) CONCAT(CONCAT(a, b), c)
#define CONCAT4(a, b, c, d) CONCAT(CONCAT3(a, b, c), d)
#define CONCAT5(a, b, c, d, e) CONCAT(CONCAT4(a, b, c, d), e)
#define CONCAT6(a, b, c, d, e, f) CONCAT(CONCAT5(a, b, c, d, e), f)
#define _WSTR(x) L##x
#define WSTR(x) _WSTR(x)

// common constants
#define NOP 0x90
#define INT3 0xCC

#define MAXLINE 4096
#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"

#define EXTERNAL_UNPACKER "PAL3unpack.dll"
#define EXTERNAL_UNPACKER_FIXED "PAL3unpack_fixed.dll"
#define ERROR_FILE "PAL3patch.log"
#define CONFIG_FILE "PAL3patch.conf"
#define MAX_CONFIG_LINES 20

// MAX_PUSH_DWORDS controls how many dwords 'asmentry' will reserve for possible stack pushes
// this value can't be too large (no more than one page)
#define MAX_PUSH_DWORDS 16

#define MAKE_PATCHSET_NAME(name) CONCAT(name, _patchset)
#define MAKE_PATCHSET(name) void MAKE_PATCHSET_NAME(name)(int flag)
#define INIT_PATCHSET(name) \
    ({ \
        int __flag = get_int_from_configfile(TOSTR(name)); \
        if (__flag) MAKE_PATCHSET_NAME(name)(__flag); \
        __flag; \
    })

#define MAKE_PATCH_PROC(funcname) void funcname(struct trapframe *tf)
#define MAKE_PATCH_NAME(prefix, addr, size) CONCAT5(prefix, _, addr, _, size)
#define MAKE_PATCH(prefix, addr, size) MAKE_PATCH_PROC(MAKE_PATCH_NAME(prefix, addr, size))
#define INIT_PATCH(prefix, addr, size, oldcode) \
    do { \
        check_code(addr, oldcode, size); \
        make_patch_proc_call(addr, MAKE_PATCH_NAME(prefix, addr, size), size); \
    } while (0)


#ifndef __ASSEMBLER__

// framework.c
extern void memcpy_to_process(unsigned dest, const void *src, unsigned size);
extern void memcpy_from_process(void *dest, unsigned src, unsigned size);
extern void make_jmp(unsigned addr, const void *jtarget);
extern void check_code(unsigned addr, const void *code, unsigned size);
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

// misc.c
#define fail(fmt, ...) __fail(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __fail(const char *file, int line, const char *func, const char *fmt, ...);

// trapframe.c
struct trapframe;
typedef void (*patch_proc_t)(struct trapframe *tf);
struct trapframe {
    union {
        struct {
            unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
        };
        struct {
            unsigned *p_edi, *p_esi, *p_ebp, *p_esp, *p_ebx, *p_edx, *p_ecx, *p_eax;
        };
    };
    unsigned ret_addr;
    patch_proc_t patch_proc;
};
extern void __push_dword(struct trapframe *tf, unsigned data);
extern unsigned __pop_dword(struct trapframe *tf);
extern void patchentry(struct trapframe *tf);
extern void make_patch_proc_call(unsigned addr, patch_proc_t patch_proc, unsigned size);
#define PUSH_DWORD(data) __push_dword((tf), (data))
#define POP_DWORD() __pop_dword((tf))
#define TOPTR(addr) ((void *)(addr))
#define MEM_DWORD(addr) (*(unsigned *)(addr))
#define MEM_WORD(addr) (*(unsigned short *)(addr))
#define MEM_BYTE(addr) (*(unsigned char *)(addr))
#define R_EAX ((tf)->eax)
#define R_ECX ((tf)->ecx)
#define R_EDX ((tf)->edx)
#define R_EBX ((tf)->ebx)
#define R_ESP ((tf)->esp)
#define R_EBP ((tf)->ebp)
#define R_ESI ((tf)->esi)
#define R_EDI ((tf)->edi)

// asmentry.S
extern void __stdcall asmentry(unsigned patch_id);

// cfgreader.c
extern void read_config_file();
extern const char *get_string_from_configfile(const char *key);
extern int get_int_from_configfile(const char *key);
extern void get_all_config(char *buf, unsigned size);

// PAL3patch.c
extern const char build_info[];

// all patchs
MAKE_PATCHSET(testcombat);
MAKE_PATCHSET(cdpatch);
MAKE_PATCHSET(regredirect);
MAKE_PATCHSET(disablekbdhook);

#endif // __ASSEMBLER__



#endif
