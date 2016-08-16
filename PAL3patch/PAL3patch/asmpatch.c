#include <windows.h>
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

static unsigned char *dyncode_page = NULL;
static unsigned dyncode_ptr;

static unsigned char *alloc_dyncode_buffer(unsigned size)
{
    if (!dyncode_page || dyncode_ptr + size > PAGE_SIZE) {
        dyncode_page = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!dyncode_page) fail("can't alloc memory for dynamic code.");
        dyncode_ptr = 0;
    }
    unsigned char *ret = dyncode_page + dyncode_ptr;
    dyncode_ptr += size;
    return ret;
}

void make_patch_proc_call(unsigned addr, patch_proc_t patch_proc, unsigned size)
{
    if (size < 5) fail("size is too small.");
    
    unsigned jmpimm;
    unsigned char *dyncode = alloc_dyncode_buffer(16);
    memcpy(dyncode, "\xFF\x34\xE4\xC7\x44\xE4\x04", 7);
    memcpy(dyncode + 7, &patch_proc, 4); // PUSH patch_id
    jmpimm = TOUINT(asmentry) - (TOUINT(dyncode) + 16);
    dyncode[11] = 0xE9; memcpy(dyncode + 12, &jmpimm, 4); // JMP asmentry
    FlushInstructionCache(GetCurrentProcess(), dyncode, 16);
    
    unsigned char *buf = malloc(size);
    memset(buf + 5, 0x90, size - 5);
    jmpimm = TOUINT(dyncode) - (addr + 5);
    buf[0] = 0xE8; memcpy(buf + 1, &jmpimm, 4);
    memcpy_to_process(addr, buf, size);
    free(buf);
}
