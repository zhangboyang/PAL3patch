// PAL3Adump 20170629

// dump PE sections after decrypt
// see notes20160712.txt for details

#include <windows.h>
#include <stdio.h>
#include <assert.h>

extern FARPROC unpack_entry;
static HMODULE unpack_base;

#define MAXLINE 4096

static char *dump_section(const char *name, unsigned vaddr, unsigned psize)
{
    static char buf[MAXLINE];
    snprintf(buf, sizeof(buf), "dump%s", name);
    FILE *fp = fopen(buf, "wb");
    assert(fp);
    int ret = fwrite((void *) vaddr, 1, psize, fp);
    assert(ret == psize);
    fclose(fp);
    snprintf(buf, sizeof(buf), "%s\t%08X\t%08X\n", name, vaddr, psize);
    return buf;
}

static void do_dump()
{
    char buf[MAXLINE];
    buf[0] = '\0';
    
    // MANUALLY enter section details here
    strcat(buf, dump_section(".text" , 0x00401000, 0x00156C57));
    strcat(buf, dump_section(".rdata", 0x00558000, 0x00017C0D));
    strcat(buf, dump_section(".data" , 0x00570000, 0x0009B000));
    strcat(buf, dump_section(".rsrc" , 0x02294000, 0x0000546A));
    
    FILE *fp = fopen("dump.summary.txt", "w");
    fprintf(fp, "%s", buf);
    fclose(fp);
    
    MessageBoxA(NULL, "Dump OK!", "PAL3dump", 0);
    exit(0);
}

static void wpm_memcpy(void *dest, void *src, size_t size)
{
    // use WriteProcessMemory to do memcpy()
    BOOL ret;
    SIZE_T written;
    ret = WriteProcessMemory(GetCurrentProcess(), dest, src, size, &written);
    assert(ret);
    assert(written == size);
}

static void make_jmp(void *addr, unsigned char opcode, void *dest)
{
    // addr: OP AA BB CC DD
    //  opcode^ ^destnation offset
    
    // next_instr + offset = dest
    unsigned long offset = dest - (addr + 5);
    unsigned char buf[5];
    buf[0] = opcode;
    memcpy(&buf[1], &offset, 4);
    wpm_memcpy(addr, buf, sizeof(buf));
}

void dump_entry()
{
    //MessageBoxA(NULL, "Hello World", "PAL3dump", 0);
    
    // load the unpacker
    unpack_base = LoadLibrary("pal3aunpack.dll");
    assert(unpack_base);
    unpack_entry = GetProcAddress(unpack_base, (LPCSTR) 1);
    assert(unpack_entry);
    
    // patch code for dump
    void *base_addr = unpack_base;
    // [CALL do_dump] after the unpacker called VirtualFree()
    // MANUALLY enter the position of patch here
    make_jmp(base_addr + 0x1246, 0xE8, do_dump);
}
