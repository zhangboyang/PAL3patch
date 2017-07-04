// PAL3Adump 20170629

// dump PE sections after decrypt
// see notes20160712.txt for details

#include <windows.h>
#include <stdio.h>
#include <assert.h>

extern FARPROC unpack_entry;
static HMODULE unpack_base;

#define MAXLINE 4096
#define fail(x) assert(!x)
#define TOPTR(addr) ((void *)(addr))
#define TOUINT(addr) ((unsigned)(addr))

void memcpy_to_process(unsigned dest, const void *src, unsigned size)
{
    // using WriteProcessMemory may failed when writing to IAT
    // use VirtualProtect instead
    DWORD flOldProtect, tmp;
    BOOL ret;
    ret = VirtualProtect((void *) dest, size, PAGE_EXECUTE_READWRITE, &flOldProtect);
    if (!ret) fail("VirtualProtect() failed.");
    memcpy((void *) dest, src, size);
    ret = VirtualProtect((void *) dest, size, flOldProtect, &tmp);
    if (!ret) fail("VirtualProtect() failed.");
    FlushInstructionCache(GetCurrentProcess(), (void *) dest, size);
}
void make_branch(unsigned addr, unsigned char opcode, const void *jtarget, unsigned size)
{
    unsigned jmpimm = (unsigned) jtarget - (addr + 5);
    if (size < 5) fail("size is to small to make a branch instuction");
    unsigned char *instrbuf = malloc(size);
    memset(instrbuf + 5, 0x90, size - 5);
    instrbuf[0] = opcode;
    memcpy(instrbuf + 1, &jmpimm, 4);
    memcpy_to_process(addr, instrbuf, size);
    free(instrbuf);
}

void make_jmp(unsigned addr, const void *jtarget)
{
    make_branch(addr, 0xE9, jtarget, 5);
}
void make_uint(unsigned addr, unsigned uint)
{
    memcpy_to_process(addr, &uint, sizeof(uint));
}
void make_pointer(unsigned addr, void *ptr)
{
    make_uint(addr, TOUINT(ptr));
}


unsigned rand_u32()
{
    return (rand() & 0xFF) + ((rand() & 0xFF) << 8) + ((rand() & 0xFF) << 16) + ((rand() & 0xFF) << 24);
}


FILE *logfp;

#define MAXVI 10000
static struct vi {
    void *page;
    unsigned size;
    int flag;
} vi[MAXVI];
static int nr_vi = 0;

LPVOID WINAPI my_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
    assert(lpAddress == NULL);
    assert(flAllocationType == MEM_COMMIT);
    assert(flProtect == PAGE_READWRITE);
    void *r = TOPTR(0x80000000 | rand_u32());
    assert(nr_vi < MAXVI);
    vi[nr_vi++] = (struct vi) {
        .page = r,
        .size = dwSize,
        .flag = 0,
    };
    fprintf(logfp, "%08X: VirtualAlloc(%08X, %08X, %08X, %08X) = %08X\n", TOUINT(__builtin_return_address(0)), TOUINT(lpAddress), TOUINT(dwSize), TOUINT(flAllocationType), TOUINT(flProtect), TOUINT(r));
    return r;
}

BOOL WINAPI my_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
    assert(dwSize == 0);
    assert(dwFreeType == MEM_RELEASE);
    int i;
    for (i = 0; i < nr_vi; i++) {
        if (vi[i].page == lpAddress && vi[i].flag == 0) {
            vi[i].flag = 1;
            break;
        }
    }
    assert(i < nr_vi);
    fprintf(logfp, "%08X: VirtualFree(%08X, %08X, %08X)\n", TOUINT(__builtin_return_address(0)), TOUINT(lpAddress), TOUINT(dwSize), TOUINT(dwFreeType));
    
    return TRUE;
}

FARPROC WINAPI my_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == GetModuleHandleA("KERNEL32.DLL")) {
        if (strcmp(lpProcName, "VirtualAlloc") == 0) {
            return TOPTR(my_VirtualAlloc);
        } else if (strcmp(lpProcName, "VirtualFree") == 0) {
            return TOPTR(my_VirtualFree);
        }
    }
    return NULL;
}

void search_page()
{
    int i;
    FILE *fp = fopen("analysis.valloc.txt", "w");
    for (i = 0; i < nr_vi; i++) {
        if (vi[i].flag != 0) continue;
        unsigned val = TOUINT(vi[i].page);
        // MANUALLY ENTER SECTION DATA HERE
        void *begin = TOPTR(0x00401000);
        void *end = TOPTR(0x0229946A);
        int cnt = 0;
        while (begin + 4 < end) {
            if (memcmp(begin, &val, 4) == 0) {
                cnt++;
                fprintf(fp, "VALLOC PTR %08X SIZE %08X\n", TOUINT(begin), vi[i].size);
            }
            begin++;
        }
        assert(cnt == 1);
    }
    fclose(fp);
}

void finish()
{
    fclose(logfp);
    search_page();
    MessageBoxA(NULL, "OK\nyou should change random seed and run again to ensure no errors.", "PAL3Atestvalloc", 0);
    TerminateProcess(GetCurrentProcess(), 0);
    while (1);
}

void testvalloc_entry()
{
    // load the unpacker
    unpack_base = LoadLibrary("pal3aunpack.dll");
    assert(unpack_base);
    unpack_entry = GetProcAddress(unpack_base, (LPCSTR) 1);
    assert(unpack_entry);
    
    // set random seed
    srand(1223);
    
    // open log file
    logfp = fopen("valloc.log.txt", "w");
    
    // patch code for valloc test
    make_pointer(TOUINT(unpack_base) + 0x1052, my_GetProcAddress);
    make_uint(TOUINT(unpack_base) + 0x10F8, TOUINT(finish) - 0x400000);
}
