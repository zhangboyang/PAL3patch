#include "common.h"

MAKE_PATCHSET(depcompatible)
{
    // we should make PAL3.EXE pages Read-Write-Execute
    void *image_base = GetModuleHandle(NULL);
    PIMAGE_DOS_HEADER pdoshdr = image_base;
    PIMAGE_NT_HEADERS pnthdr = image_base + pdoshdr->e_lfanew;
    PIMAGE_SECTION_HEADER psecthdr = image_base + pdoshdr->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    int sections = pnthdr->FileHeader.NumberOfSections;
    int i;
    for (i = 0; i < sections; i++) {
        DWORD flOldProtect;
        if (!VirtualProtect(image_base + psecthdr[i].VirtualAddress, psecthdr[i].Misc.VirtualSize, PAGE_EXECUTE_READWRITE, &flOldProtect)) {
            fail("VirtualProtect() failed, section=%.8s, vaddr=%08X, vsize=%08X", psecthdr[i].Name, image_base + psecthdr[i].VirtualAddress, psecthdr[i].Misc.VirtualSize);
        }
    }
}
