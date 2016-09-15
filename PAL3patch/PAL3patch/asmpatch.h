#ifndef PAL3PATCH_ASMPATCH_H
#define PAL3PATCH_ASMPATCH_H

// MAX_PUSH_DWORDS controls how many dwords 'asmentry' will reserve for possible stack pushes
// this value can't be too large (no more than one page)
// this value mustn't smaller than 3
#define MAX_PUSH_DWORDS 16

#ifndef __ASSEMBLER__



#define MAKE_PATCHSET_NAME(name) CONCAT(patchset_, name)
#define MAKE_PATCHSET(name) void MAKE_PATCHSET_NAME(name)(int flag)
#define GET_PATCHSET_FLAG(name) (get_int_from_configfile(TOSTR(name)))
#define INIT_PATCHSET(name) \
    ({ \
        int __flag = GET_PATCHSET_FLAG(name); \
        if (__flag) MAKE_PATCHSET_NAME(name)(__flag); \
        __flag; \
    })

#define MAKE_ASMPATCH_PROC(funcname) void funcname(struct trapframe *tf)
#define MAKE_ASMPATCH_NAME(name) CONCAT(asmpatch_, name)
#define MAKE_ASMPATCH(name) MAKE_ASMPATCH_PROC(MAKE_ASMPATCH_NAME(name))
#define INIT_ASMPATCH(name, addr, size, oldcode) \
    do { \
        check_code((addr), (oldcode), (size)); \
        make_patch_proc_call((addr), MAKE_ASMPATCH_NAME(name), (size)); \
    } while (0)

struct trapframe;
typedef void (*patch_proc_t)(struct trapframe *tf);
struct trapframe {
    unsigned char fpustate[108];
    union {
        struct {
            unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
        };
        struct {
            unsigned *p_edi, *p_esi, *p_ebp, *p_esp, *p_ebx, *p_edx, *p_ecx, *p_eax;
        };
    };
    unsigned eflags;
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
#define TOUINT(addr) ((unsigned)(addr))
#define M_DWORD(addr) (*(unsigned *)(addr))
#define M_WORD(addr) (*(unsigned short *)(addr))
#define M_BYTE(addr) (*(unsigned char *)(addr))
#define R_EAX ((tf)->eax)
#define R_ECX ((tf)->ecx)
#define R_EDX ((tf)->edx)
#define R_EBX ((tf)->ebx)
#define R_ESP ((tf)->esp)
#define R_EBP ((tf)->ebp)
#define R_ESI ((tf)->esi)
#define R_EDI ((tf)->edi)
#define RETADDR ((tf)->ret_addr)

// asmentry.S
extern void __stdcall asmentry(unsigned patch_id);



#endif
#endif
