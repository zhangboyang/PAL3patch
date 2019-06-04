#include "common.h"

void patchentry(struct trapframe *tf)
{
    unsigned old_esp = tf->esp;
    tf->patch_proc(tf);
    unsigned new_esp = tf->esp;
    if (new_esp < old_esp && old_esp - new_esp > max_push_dwords * 4) {
        fail("too many stack memory allocated.");
    }
}



void make_asmpatch_proc_call(unsigned addr, patch_proc_t patch_proc, unsigned size)
{
    if (size < 5) fail("size is too small.");
    
    unsigned jmpimm;
    unsigned char *dyncode = alloc_dyncode_buffer(16);
    memcpy(dyncode, "\xFF\x34\xE4\xC7\x44\xE4\x04", 7);
    memcpy(dyncode + 7, &patch_proc, 4); // PUSH patch_id
    jmpimm = TOUINT(asmentry) - (TOUINT(dyncode) + 16);
    dyncode[11] = 0xE9; memcpy(dyncode + 12, &jmpimm, 4); // JMP asmentry
    flush_instruction_cache(dyncode, 16);
    
    unsigned char *buf = malloc(size);
    memset(buf + 5, 0x90, size - 5);
    jmpimm = TOUINT(dyncode) - (addr + 5);
    buf[0] = 0xE8; memcpy(buf + 1, &jmpimm, 4);
    memcpy_to_process(addr, buf, size);
    free(buf);
}
