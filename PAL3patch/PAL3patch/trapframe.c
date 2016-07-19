#include <stdlib.h>
#include <string.h>
#include "common.h"

static int push_dword_count;

void __push_dword(struct trapframe *tf, unsigned data)
{
    if (push_dword_count >= MAX_PUSH_DWORDS) fail("too many stack PUSHes.");
    *--tf->p_esp = data;
    push_dword_count++;
}

unsigned __pop_dword(struct trapframe *tf)
{
    push_dword_count--;
    return *tf->p_esp++;
}

void patchentry(struct trapframe *tf)
{
    push_dword_count = 0;
    tf->patch_proc(tf);
}

void make_patch_proc_call(unsigned addr, patch_proc_t patch_proc, unsigned size)
{
    if (size < 10) fail("size is too small.");
    unsigned char *buf = malloc(size);
    memset(buf, 0x90, size); // fill NOPs
    buf[0] = 0x68; memcpy(buf + 1, &patch_proc, 4); // PUSH patch_id
    unsigned jmpimm = (unsigned) asmentry - (addr + 10);
    buf[5] = 0xE8; memcpy(buf + 6, &jmpimm, 4); // CALL asmentry
    memcpy_to_process(addr, buf, size);
    free(buf);
}
