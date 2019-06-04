#include "common.h"

// current offset of default location of GBENGINE.DLL
unsigned gboffset;

// self module handle
HINSTANCE hinstDLL;

static void self_check()
{
    // check SDK versions
    if (D3D_SDK_VERSION != 31) {
        fail("wrong D3D_SDK_VERSION.");
    }
    if (D3DX_SDK_VERSION != 21) {
        fail("wrong D3DX_SDK_VERSION.");
    }
    if (!D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION)) {
        fail("mismatched header and library.");
    }
    
    // check basic types
    assert(((wchar_t) -1) > 0); // ftfont request wchar_t is unsigned
    
    
    // check patch struct sizes
    assert(sizeof(struct uiwnd_ptag) == 4);
    
    // check pal3a struct sizes
    PAL3A_STRUCT_SELFCHECK();
}

static void init_folders()
{
    CreateDirectoryA("save", NULL);
    CreateDirectoryA("snap", NULL);
}

// init_stage1() should be called before unpacker is executed (if exists)
static void init_stage1()
{
    // do self-check
    self_check();
    
    // init gboffset
    gboffset = get_module_base("GBENGINE.DLL") - 0x10000000;
    
    // init early locale
    init_locale_early();
    
    // read config
    read_config_file();

    // must init depcompatible in stage1
    INIT_PATCHSET(depcompatible);
}

// init_stage2() should be called after EXE is unpacked
static void init_stage2()
{
    
    // init folders
    init_folders();
    
    // init memory allocators
    init_memory_allocators();
    
    // init hook framework
    init_hooks();
    init_effect_hooks();
    init_texture_hooks();
    
    // init freetype
    init_ftfont();
    
    // init_locale() must called after INIT_PATCHSET(setlocale)
    INIT_PATCHSET(setlocale);
    init_locale();
    
    // init pal3 setup path patch
    init_setpal3path();
    
    // init patchsets
    INIT_PATCHSET(cdpatch);
    INIT_PATCHSET(dpiawareness);
    INIT_PATCHSET(regredirect);
    INIT_PATCHSET(console);
    INIT_PATCHSET(relativetimer);
    INIT_PATCHSET(audiofreq);
    INIT_PATCHSET(showfps);
    INIT_PATCHSET(timerresolution);
    INIT_PATCHSET(reduceinputlatency); // should after INIT_PATCHSET(showfps)
    INIT_PATCHSET(terminateatexit);
    INIT_PATCHSET(preciseresmgr);
    INIT_PATCHSET(nocpk);
    INIT_PATCHSET(testcombat);
    INIT_PATCHSET(fixacquire);
    INIT_PATCHSET(fixattacksequen);
    
    if (INIT_PATCHSET(graphicspatch)) {
        // these are subpatchs of graphics patch
        
        INIT_PATCHSET(fixfov);
        INIT_PATCHSET(fixortho);
        INIT_PATCHSET(nolockablebackbuffer);
        INIT_PATCHSET(fixreset);
        
        if (INIT_PATCHSET(fixui)) { 
            // must called after INIT_PATCHSET(graphicspatch)
            // must called after INIT_PATCHSET(setlocale) because of D3DXCreateFont need charset information
            // ui fixes

            INIT_PATCHSET(uireplacefont);
            INIT_PATCHSET(fixpunctuation);
            INIT_PATCHSET(fixcombatui);
            INIT_PATCHSET(fix3dctrl);
            INIT_PATCHSET(fixlineupui);
            INIT_PATCHSET(fixobjectcamera);
            INIT_PATCHSET(fixcompdonateui);
            INIT_PATCHSET(fixjusticebookui);
            INIT_PATCHSET(fixcoverfrm);
            INIT_PATCHSET(fixuistaticex);
            INIT_PATCHSET(fixsceneui);
            INIT_PATCHSET(uireplacetexf);
            INIT_PATCHSET(clampuilib);
            INIT_PATCHSET(fixbigmap);
            INIT_PATCHSET(fixpropui);
        }

        INIT_PATCHSET(fixtrail);
        INIT_PATCHSET(forcesettexture);
        INIT_PATCHSET(fixeffect);
        INIT_PATCHSET(screenshot); // should after as many patches as possible
    }
    
    

    // load external plugins
    init_plugins();
    
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
    unsigned jmpaddr = unpacker_base + 0x6a88;
    check_code(jmpaddr, "\xC3\x90\x90\x90\x90", 5);
    make_jmp(jmpaddr, init_stage2);
}


// this function will be called immediately after DLL is injected by launcher
void launcher_entry(unsigned oep_addr)
{
    init_stage1();
    
    unsigned unpacker_base = (unsigned) GetModuleHandle("PAL3A.DLL");
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

BOOL WINAPI DllMain(HINSTANCE _hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    hinstDLL = _hinstDLL;
    DisableThreadLibraryCalls(hinstDLL);
    return TRUE;
}
