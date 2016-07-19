#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"



static void init_patch()
{
    read_config_file();
    INIT_PATCHSET(testcombat);
    INIT_PATCHSET(cdpatch);
    
    char buf[MAXLINE];
    sprintf(buf, "Hello World!\nasmentry = %08X\n", (unsigned) asmentry);
    MessageBox(NULL, buf, "PAL3patch", 0);
}



// patch unpacker code, let it call init_patch() after unpacking
static void patch_unpacker(unsigned unpacker_base)
{
    // there is a RETN and many NOPs at end of unpacking code
    // so replace this RETN with a JMP to our init_patch()
    // p.s. since the unpacker destoryed the register values PUSHADed
    //      we doesn't care about it, either
    unsigned jmpaddr = unpacker_base + 0x57d1;
    check_code(jmpaddr, "\xC3\x90\x90\x90\x90", 5);
    make_jmp(jmpaddr, init_patch);
}


// this function will be called immediately after DLL is injected
void init(unsigned oep_addr)
{
    unsigned unpacker_base = (unsigned) GetModuleHandle("PAL3.DLL");
    if (unpacker_base) {
        // unpacker exists and not executed
        // we should call init_patch() when after unpacking
        patch_unpacker(unpacker_base);
    } else {
        // no unpacker exists
        init_patch();
    }
}
