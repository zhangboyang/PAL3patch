// patch framework
#include <windows.h>
#include <string.h>
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
}
void memcpy_from_process(void *dest, unsigned src, unsigned size)
{
    memcpy(dest, (void *) src, size);
}

void make_jmp(unsigned addr, const void *jtarget)
{
    unsigned jmpimm = (unsigned) jtarget - (addr + 5);
    unsigned char instrbuf[5];
    instrbuf[0] = 0xE9;
    memcpy(instrbuf + 1, &jmpimm, 4);
    memcpy_to_process(addr, instrbuf, 5);
}

void check_code(unsigned addr, const void *code, unsigned size)
{
    void *buf = malloc(size);
    memcpy_from_process(buf, addr, size);
    int ret = memcmp(buf, code, size);
    if (ret != 0) fail("Code mismatch at 0x%08X.", addr);
    free(buf);
}

void *get_func_address(const char *dllname, const char *funcname)
{
    HMODULE himportmodule = GetModuleHandle(dllname);
    if (!himportmodule) fail("Can't get handle of module '%s'.", dllname);
    FARPROC pimportfunc = GetProcAddress(himportmodule, funcname);
    if (!pimportfunc) fail("Can't get address of function '%s'.", funcname);
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
    if (!pimpdesc->Name) fail("Can't find import descriptor for module '%s'.", dllname);
    void *oldptr = get_func_address(dllname, funcname);
    hook_iat(image_base + pimpdesc->FirstThunk, oldptr, newptr);
    return oldptr;
}
