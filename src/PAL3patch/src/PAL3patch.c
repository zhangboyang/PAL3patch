#include "common.h"

// current offset of default location of GBENGINE.DLL
unsigned gboffset;

// self module handle
HINSTANCE patch_hinstDLL;

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
    
    // check pal3 struct sizes
    PAL3_STRUCT_SELFCHECK();
}

static void fix_unpacker_bug()
{
    unsigned funcaddr = get_branch_jtarget(0x004229A6, 0xE8);
    if (*(unsigned char *) funcaddr == 0xE9) {
        // if there is a JMP, follow it
        funcaddr = get_branch_jtarget(funcaddr, 0xE9);
    }
    make_uint(funcaddr + 0x6B, 0x00000014);
}

static void prepare_fs()
{
    create_dir("save");
    create_dir("snap");
    
    reset_attrib("Pal3Log.txt");
    reset_attrib("EngineLog.txt");
}

static HANDLE game_mutex = NULL;
static HANDLE temp_mutex = NULL;

static void acquire_game_mutex()
{
    if (!(game_mutex = acquire_mutex("PAL3patch_GameMutex", 100))) {
        MessageBoxW(NULL, wstr_nomutex_text, wstr_nomutex_title, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
        die(0);
    }
}

void fork_laa(int laa)
{
    if (!!laa == !!is_laa()) return;
    HANDLE fork_mutex = acquire_mutex("PAL3patch_ForkMutex", 100);
    if (!fork_mutex) die(1);
    if (game_mutex) release_mutex(game_mutex);
    if (temp_mutex) release_mutex(temp_mutex);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcess((laa ? GAME_EXE_LAA : GAME_EXE), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) die(1);
    if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0) die(1);
    DWORD dwExitCode;
    if (!GetExitCodeProcess(pi.hProcess, &dwExitCode)) die(1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    die(dwExitCode);
}

static void process_temp_command()
{
    temp_mutex = acquire_mutex("PAL3patch_TempMutex", 100);
    if (!temp_mutex) die(1);
    if (file_exists(PATCH_TEMP_IN)) {
        char cmd[MAXLINE];
        FILE *fp;
        fp = robust_fopen(PATCH_TEMP_IN, "r");
        if (fp) {
            cmd[fread(cmd, 1, sizeof(cmd) - 1, fp)] = 0;
            if (ferror(fp)) cmd[0] = 0;
            fclose(fp);
        } else {
            cmd[0] = 0;
        }
        if (!str_endswith(cmd, "\n")) die(1);
        
        if (str_startswith(cmd, "d3denum ")) {
            char *config = str_rtrim(cmd + strlen("d3denum "), " \n");
            if (!prepare_d3denum(config)) die(1);
        }
        
        if (!robust_delete1(PATCH_TEMP_IN)) {
            MessageBoxW_format(NULL, wstr_cantdeltemp_text, wstr_cantdeltemp_title, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND, PATCH_TEMP_IN);
            die(1);
        }
        fp = robust_fopen(PATCH_TEMP_OUT, "w");
        if (!fp) die(1);
        int success = 0;
        
        if (str_startswith(cmd, "d3denum ")) {
            success = run_d3denum(fp);
        }
        
        fclose(fp);
        die(success ? 0 : 1);
    }
    robust_delete((const char *[]) { PATCH_TEMP_IN, PATCH_TEMP_OUT }, 2);
    release_mutex(temp_mutex); temp_mutex = NULL;
}

// init_stage1() should be called before unpacker is executed (if exists)
static void init_stage1()
{
    int flag;
    
    // do self-check
    self_check();
    
    // init gboffset
    gboffset = get_module_base("GBENGINE.DLL") - 0x10000000;
    
    // init early locale
    init_locale_early();
    
    // process temp command
    process_temp_command();
    
    // acquire mutex
    acquire_game_mutex();
    
    // read config
    read_config_file();
    
    // prepare d3d9 alternative
    prepare_d3d9_alternative();
    
    // init early patchsets in stage1
    INIT_PATCHSET(depcompatible);
    INIT_PATCHSET(dpiawareness);
}

// init_stage2() should be called after EXE is unpacked
static void init_stage2()
{
    int flag;
    
    // fix unpacker bug that would crash game when music is disabled in config.ini
    fix_unpacker_bug();
    
    // prepare filesystem environment
    prepare_fs();
    
    // init memory allocators
    init_memory_allocators();
    
    // init d3d9 alternative
    init_d3d9_alternative();
    
    // init hook framework
    init_hooks();
    init_effect_hooks();
    init_texture_hooks();
    
    // init freetype
    init_ftfont();
    
    // init_locale() must called after INIT_PATCHSET(setlocale)
    INIT_PATCHSET(setlocale);
    init_locale();
    
    // check bad game path
    check_badpath();
    
    // init patchsets
    INIT_PATCHSET(cdpatch);
    INIT_PATCHSET(regredirect);
    INIT_PATCHSET(disableime);
    INIT_PATCHSET(disablekbdhook);
    INIT_PATCHSET(terminateatexit);
    INIT_PATCHSET(timerresolution);
    INIT_PATCHSET(fixmemfree);
    INIT_PATCHSET(nocpk);
    INIT_PATCHSET(console);
    INIT_PATCHSET(relativetimer);
    INIT_PATCHSET(kahantimer);
    INIT_PATCHSET(fixlongkuiattack);
    INIT_PATCHSET(fixattacksequen);
    INIT_PATCHSET(fixhockshopbuy);
    INIT_PATCHSET(kfspeed);
    INIT_PATCHSET(fixacquire);
    INIT_PATCHSET(preciseresmgr);
    INIT_PATCHSET(audiofreq);
    INIT_PATCHSET(showfps);
    INIT_PATCHSET(reduceinputlatency); // should after INIT_PATCHSET(showfps)
    INIT_PATCHSET(testcombat);
    INIT_PATCHSET(reginstalldir);
    INIT_PATCHSET(improvearchive);
    INIT_PATCHSET(fixloading);
    INIT_PATCHSET(nommapcpk);
    INIT_PATCHSET(fixnosndcrash);
    INIT_PATCHSET(fixpathto);
    INIT_PATCHSET(fpupreserve);
    
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
            INIT_PATCHSET(fixuistaticex);
            INIT_PATCHSET(fixsceneui);
            INIT_PATCHSET(uireplacetexf);
            INIT_PATCHSET(clampuilib);
            INIT_PATCHSET(fixuibuttonex);
            INIT_PATCHSET(fixunderwater);
        }
        INIT_PATCHSET(fixeffect);
        INIT_PATCHSET(forcesettexture);
        INIT_PATCHSET(fixtrail);
        INIT_PATCHSET(screenshot);
    }
    
    // load external plugins
    init_plugins();
    
    // check game version
    check_gameversion();
    
    // check incompatible tools
    check_badtools();
    
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
    robust_delete1(EXTERNAL_UNPACKER_FIXED);
    if (!CopyFile(EXTERNAL_UNPACKER, EXTERNAL_UNPACKER_FIXED, FALSE)) return 0;
    FILE *fp = robust_fopen(EXTERNAL_UNPACKER_FIXED, "r+bc");
    if (!fp) return 0;
    if (fseek(fp, fileoffset, SEEK_SET) != 0) goto fail;
    if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf)) goto fail;
    if (memcmp(buf, oldcode, sizeof(oldcode) - 1) != 0) goto fail;
    if (fseek(fp, fileoffset, SEEK_SET) != 0) goto fail;
    if (fwrite(newcode, 1, sizeof(newcode) - 1, fp) != sizeof(newcode) - 1) goto fail;
    if (fflush(fp) != 0) goto fail;
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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        patch_hinstDLL = hinstDLL;
    }
    return TRUE;
}
