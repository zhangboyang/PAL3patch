#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"

// current offset of default location of GBENGINE.DLL
unsigned gboffset;

// init_stage1() should be called before unpacker is executed (if exists)
static void init_stage1()
{
    if (D3D_SDK_VERSION != 31) fail("your compiler has wrong DirectX SDK version!");
    
    gboffset = get_module_base("GBENGINE.DLL") - 0x10000000;
    read_config_file();
    
    INIT_PATCHSET(depcompatible);
}

// init_stage2() should be called after EXE is unpacked
static void init_stage2()
{
    init_gameloop_hook();
    init_atexit_hook();
    
    if (!INIT_PATCHSET(testcombat)) {
        // here are some patches not compatiable with 'testcombat'
        INIT_PATCHSET(dpiawareness);
        if (INIT_PATCHSET(graphicspatch)) {
            // these are subpatchs of graphics patch
            INIT_PATCHSET(windowed);
            INIT_PATCHSET(fixfov);
            INIT_PATCHSET(nolockablebackbuffer);
            INIT_PATCHSET(fixreset);
        }
    }
    
    INIT_PATCHSET(cdpatch);
    INIT_PATCHSET(regredirect);
    INIT_PATCHSET(disablekbdhook);
    INIT_PATCHSET(setlocale);
    INIT_PATCHSET(powersave);
    INIT_PATCHSET(timerresolution);
    INIT_PATCHSET(fixmemfree);

    show_about();
}





// patch unpacker code, let it call init_stage2() after unpacking
static void patch_unpacker(unsigned unpacker_base)
{
    // there is a RETN and many NOPs at end of unpacking code
    // so replace this RETN with a JMP to our init_stage2()
    // p.s. since the unpacker destoryed the register values PUSHADed
    //      we doesn't care about it, either
    unsigned jmpaddr = unpacker_base + 0x57d1;
    check_code(jmpaddr, "\xC3\x90\x90\x90\x90", 5);
    make_jmp(jmpaddr, init_stage2);
}


// this function will be called immediately after DLL is injected by launcher
void launcher_entry(unsigned oep_addr)
{
    init_stage1();
    
    unsigned unpacker_base = (unsigned) GetModuleHandle("PAL3.DLL");
    if (unpacker_base) {
        // unpacker exists and not executed
        // we should call init_stage2() when after unpacking
        patch_unpacker(unpacker_base);
    } else {
        // no unpacker exists
        init_stage2();
    }
}

// fix unpacker entry for Windows 98
// see notes20160721.txt for details
static int try_fix_unpacker()
{
    const unsigned fileoffset = 0x2c6;
    const unsigned char oldcode[] = "\x83\x7C\x24\x08\x01\x75\x10\x8B\x44\x24\x04";
    const unsigned char newcode[] = "\x59\x58\x5A\x83\xFA\x01\x5A\x51\x75\x0D\x90";
    unsigned char buf[sizeof(oldcode) - 1];
    if (!CopyFile(EXTERNAL_UNPACKER, EXTERNAL_UNPACKER_FIXED, FALSE)) return 0;
    FILE *fp = fopen(EXTERNAL_UNPACKER_FIXED, "r+b");
    if (!fp) return 0;
    if (fseek(fp, fileoffset, SEEK_SET) != 0) goto fail;
    if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf)) goto fail;
    if (memcmp(buf, oldcode, sizeof(oldcode) - 1) != 0) goto fail;
    if (fseek(fp, fileoffset, SEEK_SET) != 0) goto fail;
    if (fwrite(newcode, 1, sizeof(newcode) - 1, fp) != sizeof(newcode) - 1) goto fail;
    fclose(fp);
    return 1;
fail:
    fclose(fp);
    return 0;
}

unsigned sforce_unpacker_init()
{
    init_stage1();
    
    // our DLL is loaded as fake unpacker
    // we should load the real unpacker, patch it, and execute it
    HMODULE unpacker = LoadLibrary(EXTERNAL_UNPACKER);
    if (!unpacker && try_fix_unpacker()) unpacker = LoadLibrary(EXTERNAL_UNPACKER_FIXED);
    if (!unpacker) fail("Can't load unpacker.");
    patch_unpacker((unsigned) unpacker);
    
    // we should return unpacker entry address to out asm code
    return (unsigned) GetProcAddress(unpacker, (LPCSTR) 1);
}
