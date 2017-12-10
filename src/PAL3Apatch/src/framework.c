// patch framework
#include "common.h"


void memcpy_to_process(unsigned dest, const void *src, unsigned size)
{
    /*// check dest address, debug purpose only
    // should rebase and make sure GBENGINE.DLL loaded at 0x40000000
    if (0x10000000 <= dest && dest < 0x1013D000) {
        fail("invalid dest address, dest = %08X, src = %p, size = %08X.", dest, src, size);
    }*/
    
    // using WriteProcessMemory may failed when writing to IAT
    // use VirtualProtect instead
    DWORD flOldProtect, tmp;
    BOOL ret;
    ret = VirtualProtect(TOPTR(dest), size, PAGE_EXECUTE_READWRITE, &flOldProtect);
    if (!ret) fail("VirtualProtect() failed.");
    memcpy(TOPTR(dest), src, size);
    ret = VirtualProtect(TOPTR(dest), size, flOldProtect, &tmp);
    if (!ret) fail("VirtualProtect() failed.");
    flush_instruction_cache(TOPTR(dest), size);
}
void memcpy_from_process(void *dest, unsigned src, unsigned size)
{
    memcpy(dest, TOPTR(src), size);
}

unsigned get_branch_jtarget(unsigned addr, unsigned char opcode)
{
    // assume instr length is 5
    unsigned char buf[5];
    memcpy_from_process(buf, addr, 5);
    if (buf[0] != opcode) fail("jtarget opcode mismatch");
    unsigned off;
    memcpy(&off, &buf[1], 4);
    return off + addr + 5;
}

void make_branch(unsigned addr, unsigned char opcode, const void *jtarget, unsigned size)
{
    unsigned jmpimm = (unsigned) jtarget - (addr + 5);
    if (size < 5) fail("size is to small to make a branch instuction");
    unsigned char *instrbuf = malloc(size);
    memset(instrbuf + 5, 0x90, size - 5);
    instrbuf[0] = opcode;
    memcpy(instrbuf + 1, &jmpimm, 4);
    memcpy_to_process(addr, instrbuf, size);
    free(instrbuf);
}

void make_jmp(unsigned addr, const void *jtarget)
{
    make_branch(addr, 0xE9, jtarget, 5);
}
void make_call(unsigned addr, const void *jtarget)
{
    make_branch(addr, 0xE8, jtarget, 5);
}
void make_wrapper_branch(unsigned addr, const void *jtarget)
{
    unsigned char opcode0, opcode1;
    
    // fetch first byte
    memcpy_from_process(&opcode0, addr, 1);
    
    // decode first byte
    switch (opcode0) {
        // one-byte instruction
        case 0xE9: make_jmp(addr, jtarget); break;
        case 0xE8: make_call(addr, jtarget); break;
        
        // multi-byte instruction
        case 0xFF:
            // fetch second byte
            memcpy_from_process(&opcode1, addr + 1, 1);
            
            // decode second byte
            switch ((opcode0 << 8) + opcode1) { // constuct as big-endian for readability
                // two byte instruction
                case 0xFF15: make_branch(addr, 0xE8, jtarget, 6); break;

                // unknown second byte
                default: fail("unknown branch opcode %02X %02X.", opcode0, opcode1);
            }
            
            break;
        
        // unknown first byte
        default: fail("unknown branch opcode %02X.", opcode0);
    }
}
void make_wrapper_branch_batch(unsigned *addr_list, int count, const void *jtarget)
{
    while (count--) make_wrapper_branch(*addr_list++, jtarget);
}

void make_uint(unsigned addr, unsigned uint)
{
    memcpy_to_process(addr, &uint, sizeof(uint));
}

void make_pointer(unsigned addr, void *ptr)
{
    make_uint(addr, TOUINT(ptr));
}

void check_code(unsigned addr, const void *code, unsigned size)
{
    if (!code) return;
    void *buf = malloc(size);
    memcpy_from_process(buf, addr, size);
    int ret = memcmp(buf, code, size);
    if (ret != 0) fail("code mismatch at 0x%08X.", addr);
    free(buf);
}

unsigned get_module_base(const char *modulename)
{
    HMODULE hmodule = GetModuleHandle(modulename);
    if (!hmodule) fail("can't get base address of module '%s'.", modulename);
    return TOUINT(hmodule);
}

void *get_func_address(const char *dllname, const char *funcname)
{
    HMODULE himportmodule = GetModuleHandle(dllname);
    if (!himportmodule) fail("can't get handle of module '%s'.", dllname);
    FARPROC pimportfunc = GetProcAddress(himportmodule, funcname);
    if (!pimportfunc) fail("can't get address of function '%s'.", funcname);
    return pimportfunc;
}

void hook_iat(void *iatbase, void *oldptr, void *newptr)
{
    // find and hook iat item
    unsigned cnt = 0;
    void **piatitem = iatbase;
    for (; *piatitem; piatitem++) {
        if (*piatitem == oldptr) {
            memcpy_to_process((unsigned) piatitem, &newptr, sizeof(newptr));
            cnt++;
        }
    }
    if (cnt != 1) {
        warning("unable to find iat item %08X, iat hook failed.", oldptr);
    }
}

void *hook_import_table(void *image_base, const char *dllname, const char *funcname, void *newptr)
{
    // hook import table of module 'image_base'
    // return value is original function ptr
    void *oldptr = get_func_address(dllname, funcname);
    if (!oldptr) {
        fail("can't find address of %s in dll %s.", funcname, dllname);
    }
    if (image_base == GetModuleHandle(NULL)) {
        // we must hardcode IAT address since PAL3.EXE is packed
        void *iatbase = NULL;
        if (stricmp(dllname, "KERNEL32.DLL") == 0) {
            iatbase = PAL3A_KERNEL32_IATBASE;
        } else if (stricmp(dllname, "WINMM.DLL") == 0) {
            iatbase = PAL3A_WINMM_IATBASE;
        } else if (stricmp(dllname, "USER32.DLL") == 0) {
            iatbase = PAL3A_USER32_IATBASE;
        } else {
            warning("iatbase unknown for dll %s.", dllname);
        }
        if (iatbase) {
            hook_iat(iatbase, oldptr, newptr);
        }
    } else {
        PIMAGE_DOS_HEADER pdoshdr = image_base;
        PIMAGE_NT_HEADERS pnthdr = PTRADD(image_base, pdoshdr->e_lfanew);
        PIMAGE_IMPORT_DESCRIPTOR pimpdesc;
        IMAGE_IMPORT_DESCRIPTOR zeroimpdesc;
        memset(&zeroimpdesc, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        for (pimpdesc = PTRADD(image_base, pnthdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
            memcmp(pimpdesc, &zeroimpdesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && stricmp(PTRADD(image_base, pimpdesc->Name), dllname);
            pimpdesc++);
        if (!pimpdesc->Name) {
            warning("can't find import descriptor for dll '%s' in module %p, iat hook failed.", dllname, image_base);
        } else {
            hook_iat(PTRADD(image_base, pimpdesc->FirstThunk), oldptr, newptr);
        }
    }
    return oldptr;
}


#define DYNCODE_PAGESIZE 4096
static void *dyncode_page = NULL;
static unsigned dyncode_ptr;
void *alloc_dyncode_buffer(unsigned size)
{
    if (!size || size > DYNCODE_PAGESIZE) fail("invalid size %08X.", size);
    if (!dyncode_page || dyncode_ptr + size > DYNCODE_PAGESIZE) {
        dyncode_page = VirtualAlloc(NULL, DYNCODE_PAGESIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!dyncode_page) fail("can't alloc memory for dynamic code.");
        dyncode_ptr = 0;
    }
    void *ret = PTRADD(dyncode_page, dyncode_ptr);
    dyncode_ptr += size;
    return ret;
}

// add code to dyncode buffer
// there must be at least 5 bytes at patchaddr
void add_dyncode_with_jmpback(unsigned patchaddr, unsigned jmpback, void *code, unsigned size)
{
    if (patchaddr < jmpback && jmpback < patchaddr + 5) {
        fail("no space for patchaddr.");
    }
    
    void *buf = alloc_dyncode_buffer(size + 5);
    memcpy(buf, code, size);
    make_jmp(TOUINT(PTRADD(buf, size)), TOPTR(jmpback));
    flush_instruction_cache(buf, size + 5);

    make_jmp(patchaddr, buf);
}

void flush_instruction_cache(void *base, unsigned size)
{
    FlushInstructionCache(GetCurrentProcess(), base, size);
}

// copy vftable, will alloc memory and dyncode
void *dup_vftable(void *vftable, unsigned size)
{
    assert(size % 4 == 0);
    size /= 4;

    void **vtbl = malloc(size * 4);
    
    unsigned i;
    for (i = 0; i < size; i++) {
        void *entry = PTRADD(vftable, i * 4);
        
        void *codebuf = alloc_dyncode_buffer(6);
        memcpy(codebuf, "\xFF\x25", 2);
        memcpy(PTRADD(codebuf, 2), &entry, 4);
        flush_instruction_cache(codebuf, 6);
        
        vtbl[i] = codebuf;
    }
    
    return vtbl;
}
