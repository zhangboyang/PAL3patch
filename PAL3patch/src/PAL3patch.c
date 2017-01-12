#include "common.h"

// current offset of default location of GBENGINE.DLL
unsigned gboffset;

static void self_check()
{
    // asserts
    assert(D3D_SDK_VERSION == 31);
    assert(sizeof(struct uiwnd_ptag) == 4);
    assert(sizeof(struct CCBUI) == 0xA48);
    assert(sizeof(struct UIStatic) == 0x98);
    assert(sizeof(struct CCBLineupWindow) == 0x7C);
    assert(sizeof(struct UI3DCtrl) == 0x134);
    assert(sizeof(struct gbCamera) == 0x178);
    assert(sizeof(struct LineupUI) == 0x29CEC);
    assert(sizeof(struct ObjectCamera) == 0x7C);
    assert(sizeof(struct UIStaticEXA) == 0xC0);
    assert(sizeof(struct UICaption) == 0x60);
    assert(sizeof(struct UINote) == 0x10C);
    assert(sizeof(struct ScreenEffect) == 0x18);
    assert(sizeof(struct UIRoleSelect) == 0x5D0);
    assert(sizeof(struct UIHeadDialog) == 0x160);
    assert(sizeof(struct UIRoleDialog) == 0x454);
    assert(sizeof(struct UIChatRest) == 0x252C);
    assert(sizeof(struct UIEmote) == 0x8E0);
    assert(sizeof(struct UIGameFrm) == 0x6E1C);
    assert(sizeof(struct UISceneMap) == 0x3F4);
    assert(sizeof(struct UISceneFace) == 0x6F4);
    assert(sizeof(struct UISceneFrm) == 0xB30);
}

// init_stage1() should be called before unpacker is executed (if exists)
static void init_stage1()
{
    self_check();
    gboffset = get_module_base("GBENGINE.DLL") - 0x10000000;
    
#ifndef USE_MSVC_LINKER
    d3dx9_dynlink();
#endif

    read_config_file();
    sha1_init();
    add_atexit_hook(sha1_cleanup);

    MessageBox(NULL, "stage1", "stage1", 0);
    INIT_PATCHSET(depcompatible);
}

// init_stage2() should be called after EXE is unpacked
static void init_stage2()
{
    // init system_codepage
    // PATCHSET 'setlocale' may overwrite target_codepage
    target_codepage = system_codepage = GetACP();
    
    init_hooks();
    
    INIT_PATCHSET(cdpatch);
    INIT_PATCHSET(regredirect);
    INIT_PATCHSET(disablekbdhook);
    INIT_PATCHSET(setlocale);
    INIT_PATCHSET(timerresolution);
    INIT_PATCHSET(fixmemfree);
    INIT_PATCHSET(nocpk);
    INIT_PATCHSET(showfps);
    INIT_PATCHSET(console);
    INIT_PATCHSET(relativetimer);
    
    if (!INIT_PATCHSET(testcombat)) {
        // here are some patches not compatiable with 'testcombat'
        INIT_PATCHSET(dpiawareness);
        if (INIT_PATCHSET(graphicspatch)) {
            // these are subpatchs of graphics patch
            INIT_PATCHSET(fixfov);
            INIT_PATCHSET(fixortho);
            INIT_PATCHSET(nolockablebackbuffer);
            INIT_PATCHSET(reduceinputlatency); // should after INIT_PATCHSET(showfps)
            INIT_PATCHSET(fixreset);
            if (INIT_PATCHSET(fixui)) { 
                // must called after INIT_PATCHSET(graphicspatch)
                // must called after INIT_PATCHSET(setlocale) because of D3DXCreateFont need charset information
                // ui fixes
                INIT_PATCHSET(uireplacefont);
                //INIT_PATCHSET(fixloadingfrm);
                INIT_PATCHSET(fixcombatui);
                //INIT_PATCHSET(fixroledialog);
                //INIT_PATCHSET(fixgameover);
                INIT_PATCHSET(fix3dctrl);
                INIT_PATCHSET(fixlineupui);
                INIT_PATCHSET(fixuistaticex);
                INIT_PATCHSET(fixsceneui);
            }
            INIT_PATCHSET(replacetexture);
        }
    }


    // init_locale() must called after INIT_PATCHSET(setlocale)
    init_locale();
    
    // show_about() must called after init_locale()
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

// fix unpacker entry for Windows 98/Me
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
    if (!unpacker) fail("can't load external unpacker '%s'.", EXTERNAL_UNPACKER);
    patch_unpacker((unsigned) unpacker);
    
    // we should return unpacker entry address to out asm code
    return (unsigned) GetProcAddress(unpacker, (LPCSTR) 1);
}
