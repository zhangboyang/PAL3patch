// patch framework
#include <windows.h>
#include <string.h>
#include "common.h"


void memcpy_to_process(unsigned dest, void *src, unsigned size)
{
    BOOL ret = WriteProcessMemory(GetCurrentProcess(), (void *) dest, src, size, NULL);
    if (!ret) fail("WriteProcessMemory() failed.");
}
void memcpy_from_process(void *dest, unsigned src, unsigned size)
{
    BOOL ret = ReadProcessMemory(GetCurrentProcess(), (void *) src, dest, size, NULL);
    if (!ret) fail("ReadProcessMemory() failed.");
}

void make_jmp(unsigned addr, void *jtarget)
{
    unsigned jmpimm = (unsigned) jtarget - (addr + 5);
    unsigned char instrbuf[5];
    instrbuf[0] = 0xE9;
    memcpy(instrbuf + 1, &jmpimm, 4);
    memcpy_to_process(addr, instrbuf, 5);
}
