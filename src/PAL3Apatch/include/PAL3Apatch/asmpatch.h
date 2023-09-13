#ifndef PAL3APATCH_ASMPATCH_H
#define PAL3APATCH_ASMPATCH_H
// PATCHAPI DEFINITIONS


#define MAKE_ASMPATCH_PROC(funcname) void funcname(struct trapframe *tf)
#define MAKE_ASMPATCH_NAME(name) CONCAT(asmpatch_, name)
#define MAKE_ASMPATCH(name) MAKE_ASMPATCH_PROC(MAKE_ASMPATCH_NAME(name))
#define INIT_ASMPATCH(name, addr, size, oldcode) \
    do { \
        check_code((addr), (oldcode), (size)); \
        make_asmpatch_proc_call((addr), MAKE_ASMPATCH_NAME(name), (size)); \
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
    unsigned retn_addr;
    patch_proc_t patch_proc;
};

extern PATCHAPI void make_asmpatch_proc_call(unsigned addr, patch_proc_t patch_proc, unsigned size);
#define PUSH_DWORD(data) do { unsigned data_ = (data); *--(tf)->p_esp = data_; } while (0)
#define POP_DWORD() (*(tf)->p_esp++)
#define M_FLOAT(addr) (*(float *)(addr))
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
#define RETNADDR ((tf)->retn_addr)

// these helpers must be call at end of asmpatch
#define LINK_CALL(addr) do { unsigned addr_ = (addr); PUSH_DWORD(RETNADDR); RETNADDR = addr_; } while (0)
#define LINK_JMP(addr) do { RETNADDR = (addr); } while (0)
#define LINK_RETN(arg_bytes) do { unsigned arg_bytes_ = (arg_bytes); RETNADDR = POP_DWORD(); R_ESP += arg_bytes_; } while (0)


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

extern void patchentry(struct trapframe *tf);

// asmentry.S
extern unsigned max_push_dwords;
extern void __stdcall asmentry(unsigned patch_id);

#endif
#endif
