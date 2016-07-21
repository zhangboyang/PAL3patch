#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"

const char build_info[] =
    "  built on: " __DATE__ ", " __TIME__ "\n"
    "  compiler: "
#if defined(__GNUC__) && defined(__VERSION__)
        "gcc " __VERSION__
#else
        "unknown C compiler"
#endif
    "\n";


static void show_about()
{
    int flag = get_int_from_configfile("showabout");
    if (flag) {
        char cfgbuf[MAXLINE];
        get_all_config(cfgbuf, sizeof(cfgbuf));
        wchar_t buf[MAXLINE];
        snwprintf(buf, sizeof(buf) / sizeof(wchar_t), 
            L"欢迎使用《仙剑奇侠传三》增强补丁\n"
            L"\n"
            L"本补丁可以修复一些游戏程序编写不合理的地方\n"
            L"并且添加了一些实用的小功能\n" 
            L"详细信息和使用方法请参见 PAL3patch_README.txt\n" 
            L"\n"
            L"\n"
            L"配置选项:\n%hs" 
            L"源代码:\n"
            L"  https://github.com/zhangboyang/PAL3patch\n"
            L"编译信息:\n%hs"
            L"\n"
            L"\n"
            L"如果您不想每次启动时看到此信息\n"
            L"请将配置文件 %hs 中的\n"
            L"  showabout=%d\n"
            L"修改为\n"
            L"  showabout=0\n"
            L"\n"
            , cfgbuf, build_info, CONFIG_FILE, flag); 
            
            
        MessageBoxW(NULL, buf, L"关于", MB_ICONINFORMATION); 
    }
}

static void init_patch()
{
    read_config_file();
    
    show_about();
    
    INIT_PATCHSET(cdpatch);
    INIT_PATCHSET(regredirect);
    INIT_PATCHSET(disablekbdhook);
    
    if (!INIT_PATCHSET(testcombat)) {
        // here are some patches not compatiable with 'testcombat'
    }
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


// this function will be called immediately after DLL is injected by launcher
void launcher_entry(unsigned oep_addr)
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
    // our DLL is loaded as fake unpacker
    // we should load the real unpack, patch it, and execute it
    HMODULE unpacker = LoadLibrary(EXTERNAL_UNPACKER);
    if (!unpacker && try_fix_unpacker()) unpacker = LoadLibrary(EXTERNAL_UNPACKER_FIXED);
    if (!unpacker) fail("Can't load unpacker.");
    patch_unpacker((unsigned) unpacker);
    
    // we should return unpacker entry address to out asm code
    return (unsigned) GetProcAddress(unpacker, (LPCSTR) 1);
}
