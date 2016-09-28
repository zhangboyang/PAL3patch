// patch framework
#include "common.h"


void memcpy_to_process(unsigned dest, const void *src, unsigned size)
{
    // using WriteProcessMemory may failed when writing to IAT
    // use VirtualProtect instead
    DWORD flOldProtect, tmp;
    BOOL ret;
    ret = VirtualProtect((void *) dest, size, PAGE_EXECUTE_READWRITE, &flOldProtect);
    if (!ret) fail("VirtualProtect() failed.");
    memcpy((void *) dest, src, size);
    ret = VirtualProtect((void *) dest, size, flOldProtect, &tmp);
    if (!ret) fail("VirtualProtect() failed.");
    FlushInstructionCache(GetCurrentProcess(), (void *) dest, size);
}
void memcpy_from_process(void *dest, unsigned src, unsigned size)
{
    memcpy(dest, (void *) src, size);
}

void *get_branch_jtarget(unsigned addr, unsigned char opcode)
{
    // assume instr length is 5
    unsigned char buf[5];
    memcpy_from_process(buf, addr, 5);
    if (buf[0] != opcode) fail("jtarget opcode mismatch");
    void *off;
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
    if (cnt != 1) fail("unable to find iat item %08X", oldptr);
}

void *hook_import_table(void *image_base, const char *dllname, const char *funcname, void *newptr)
{
    // hook import table of module 'image_base'
    // return value is original function ptr
    PIMAGE_DOS_HEADER pdoshdr = image_base;
    PIMAGE_NT_HEADERS pnthdr = image_base + pdoshdr->e_lfanew;
    PIMAGE_IMPORT_DESCRIPTOR pimpdesc;
    IMAGE_IMPORT_DESCRIPTOR zeroimpdesc;
    memset(&zeroimpdesc, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
    for (pimpdesc = image_base + pnthdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress; memcmp(pimpdesc, &zeroimpdesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && stricmp(image_base + pimpdesc->Name, dllname); pimpdesc++);
    if (!pimpdesc->Name) fail("can't find import descriptor for module '%s'.", dllname);
    void *oldptr = get_func_address(dllname, funcname);
    hook_iat(image_base + pimpdesc->FirstThunk, oldptr, newptr);
    return oldptr;
}
