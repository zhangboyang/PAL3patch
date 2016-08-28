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
#define PAGE_SIZE 4096


#define MAXLINE 4096
#define MAXLINEFMT "%" TOSTR(MAXLINE) "s"


#define PAL3PATCH_VERSION "v1.0 alpha"
#define EXTERNAL_UNPACKER "PAL3unpack.dll"
#define EXTERNAL_UNPACKER_FIXED "PAL3unpack_fixed.dll"
#define ERROR_FILE "PAL3patch.log"
#define WARNING_FILE "PAL3patch.warning.log"
#define CONFIG_FILE "PAL3patch.conf"
#define MAX_CONFIG_LINES 100
#define MAX_GAMELOOP_HOOKS 50
#define MAX_ATEXIT_HOOKS 50

// MAX_PUSH_DWORDS controls how many dwords 'asmentry' will reserve for possible stack pushes
// this value can't be too large (no more than one page)
// this value mustn't smaller than 3
#define MAX_PUSH_DWORDS 16





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


#ifndef __ASSEMBLER__
// common includes
#include <wchar.h>

// framework.c
extern void memcpy_to_process(unsigned dest, const void *src, unsigned size);
extern void memcpy_from_process(void *dest, unsigned src, unsigned size);
extern void make_branch(unsigned addr, unsigned char opcode, const void *jtarget, unsigned size);
extern void make_jmp(unsigned addr, const void *jtarget);
extern void make_call(unsigned addr, const void *jtarget);
extern void check_code(unsigned addr, const void *code, unsigned size);
extern unsigned get_module_base(const char *modulename);
extern void *get_func_address(const char *dllname, const char *funcname);
extern void hook_iat(void *iatbase, void *oldptr, void *newptr);
extern void *hook_import_table(void *image_base, const char *dllname, const char *funcname, void *newptr);
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
extern int is_win9x();
#define fail(fmt, ...) __fail(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __attribute__((noreturn)) __fail(const char *file, int line, const char *func, const char *fmt, ...);
#define warning(fmt, ...) __warning(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __warning(const char *file, int line, const char *func, const char *fmt, ...);
extern int str2int(const char *valstr);
extern double str2double(const char *valstr);

// about.c
extern const char build_info[];
extern void show_about();

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

// cfgreader.c
extern void read_config_file();
extern const char *get_string_from_configfile(const char *key);
extern int get_int_from_configfile(const char *key);
extern void get_all_config(char *buf, unsigned size);

// PAL3patch.c
extern const char *unpacker_module_name;
extern unsigned gboffset;

// gameloophook.c
typedef void (*gameloop_func_t)(int flag);
extern void add_gameloop_hook(gameloop_func_t funcptr);
extern void call_gameloop_hooks(int flag);
extern void init_gameloop_hook();
enum game_loop_type {
    GAMELOOP_NORMAL,
    GAMELOOP_SLEEP,
    GAMELOOP_MOVIE,
    GAMELOOP_DEVICELOST,
};

// atexithook.c
extern void add_atexit_hook(void (*funcptr)(void));
extern void init_atexit_hook();

// locale.c
extern unsigned system_codepage;
extern unsigned target_codepage;
extern void init_locale();
extern const wchar_t *wstr_about_title;
extern const wchar_t *wstr_about_text;
extern const wchar_t *wstr_confirmquit_text;
extern const wchar_t *wstr_confirmquit_title;

// all patchs
MAKE_PATCHSET(testcombat);
MAKE_PATCHSET(cdpatch);
MAKE_PATCHSET(regredirect);
MAKE_PATCHSET(disablekbdhook);
MAKE_PATCHSET(depcompatible);
MAKE_PATCHSET(setlocale);
MAKE_PATCHSET(dpiawareness);
MAKE_PATCHSET(powersave);
MAKE_PATCHSET(timerresolution);
MAKE_PATCHSET(fixmemfree);

MAKE_PATCHSET(graphicspatch);
    extern int game_width, game_height;
    MAKE_PATCHSET(windowed);
    MAKE_PATCHSET(fixfov);
    MAKE_PATCHSET(nolockablebackbuffer);
    MAKE_PATCHSET(fixreset);

#endif // __ASSEMBLER__



#endif
