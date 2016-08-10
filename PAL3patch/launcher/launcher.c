// create a process and inject DLL to it
//   after inject, DLL_FUNC will be called
//     void __cdecl DLL_FUNC(unsigned entry_addr)

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef USE_UNPACKED_EXE
#define EXE_NAME "PAL3unpacked.exe"
#else
#define EXE_NAME "PAL3.exe"
#endif
#define DLL_NAME "PAL3patch.dll"
#define DLL_FUNC "launcher_entry"


/*
    remote code:
        
    ENTRY:
        jmp RENTRY (5 bytes)
    
    RENTRY:
        sub esp, 4
        pusha
        
        mov ebx, data
        call [ebx+GetCurrentProcess] // restore code at ENTRY
        push 0
        push [ebx+entry_orig_datasize]
        lea ecx, [ebx+entry_orig_data]
        push ecx
        push [ebx+entry_addr]
        push eax
        call WriteProcessMemory
        
        lea ecx, [ebx+dllname] // load our DLL
        push ecx
        call LoadLibrary
        lea [ebx+dllfunc]
        psuh ecx
        push eax
        call GetProcAddress
        push [ebx+entry_addr]
        call eax
        add esp, 4
        
        mov eax, [ebp+entry_addr] // set retn addr to ENTRY
        mov [esp+20], eax
        
        popa
        retn
    
    00401000      83EC 04       SUB ESP,4
    00401003      60            PUSHAD
    00401004      BB 00000000   MOV EBX,0
    00401009      FF53 10       CALL DWORD PTR DS:[EBX+10]
    0040100C      6A 00         PUSH 0
    0040100E      FF73 04       PUSH DWORD PTR DS:[EBX+4]
    00401011      8D4B 08       LEA ECX,[EBX+8]
    00401014      51            PUSH ECX
    00401015      FF33          PUSH DWORD PTR DS:[EBX]
    00401017      50            PUSH EAX
    00401018      FF53 14       CALL DWORD PTR DS:[EBX+14]
    0040101B      8D4B 20       LEA ECX,[EBX+20]
    0040101E      51            PUSH ECX
    0040101F      FF53 18       CALL DWORD PTR DS:[EBX+18]
    00401022      8D4B 50       LEA ECX,[EBX+50]
    00401025      51            PUSH ECX
    00401026      50            PUSH EAX
    00401027      FF53 1C       CALL DWORD PTR DS:[EBX+1C]
    0040102A      FF33          PUSH DWORD PTR DS:[EBX]
    0040102C      FFD0          CALL EAX
    0040102E      83C4 04       ADD ESP,4
    00401031      8B03          MOV EAX,DWORD PTR DS:[EBX]
    00401033      8944E4 20     MOV DWORD PTR SS:[ESP+20],EAX
    00401037      61            POPAD
    00401038      C3            RETN

*/

struct entry_code_data {
    unsigned entry_addr;               // +0x00
    unsigned entry_orig_datasize;      // +0x04
    unsigned char entry_orig_data[8];  // +0x08
    unsigned GetCurrentProcess;        // +0x10
    unsigned WriteProcessMemory;       // +0x14
    
    unsigned LoadLibrary;              // +0x18
    unsigned GetProcAddress;           // +0x1C
    
    unsigned char dllname[0x30];       // +0x20
    unsigned char dllfunc[0x30];       // +0x50
}; // size 0x80



#define ENTRY_CODE_SIZE 0x40
#define ENTRY_CODE_DATA_SIZE 0x80

#define ENTRY_CODE_DATA_OFFSET 0x5
static const char entry_code[ENTRY_CODE_SIZE] =
    "\x83\xEC\x04\x60\xBB\x00\x00\x00\x00\xFF\x53\x10\x6A\x00\xFF\x73"
    "\x04\x8D\x4B\x08\x51\xFF\x33\x50\xFF\x53\x14\x8D\x4B\x20\x51\xFF"
    "\x53\x18\x8D\x4B\x50\x51\x50\xFF\x53\x1C\xFF\x33\xFF\xD0\x83\xC4"
    "\x04\x8B\x03\x89\x44\xE4\x20\x61\xC3";

static unsigned pe_entry;
static HANDLE rprocess, rthread;

#define MAXLINE 4096
#define fail(fmt, ...) __fail(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
static void __fail(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    int len;
    snprintf(buf, sizeof(buf), "FILE: %s\nLINE: %d\nFUNC: %s\n\n", file, line, func);
    len = strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    MessageBoxA(NULL, buf, NULL, MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 1);
    va_end(ap);
}

static unsigned malloc_remote(unsigned size)
{
    return (unsigned) VirtualAllocEx(rprocess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}
static void memcpy_to_remote(unsigned dest, void *src, unsigned size)
{
    BOOL ret = WriteProcessMemory(rprocess, (void *) dest, src, size, NULL);
    if (!ret) fail("WriteProcessMemory() failed.");
}
static void memcpy_from_remote(void *dest, unsigned src, unsigned size)
{
    BOOL ret = ReadProcessMemory(rprocess, (void *) src, dest, size, NULL);
    if (!ret) fail("ReadProcessMemory() failed.");
}
static void start_execute()
{
    ResumeThread(rthread);
}
static void clean_up()
{
    CloseHandle(rprocess);
    CloseHandle(rthread);
}

static void find_pe_entry()
{
    size_t sret;
    int iret;
    IMAGE_DOS_HEADER doshdr;
    IMAGE_NT_HEADERS nthdr;
    FILE *fp = fopen(EXE_NAME, "rb");
    if (!fp) fail("Can't open file '%s'.", EXE_NAME);
    sret = fread(&doshdr, sizeof(IMAGE_DOS_HEADER), 1, fp);
    if (sret != 1) fail("Can't read IMAGE_DOS_HEADER.");
    iret = fseek(fp, doshdr.e_lfanew, SEEK_SET);
    if (iret != 0) fail("Can't seek to %08X.", doshdr.e_lfanew);
    sret = fread(&nthdr, sizeof(IMAGE_NT_HEADERS), 1, fp);
    if (sret != 1) fail("Can't read IMAGE_NT_HEADERS.");
    pe_entry = nthdr.OptionalHeader.ImageBase + nthdr.OptionalHeader.AddressOfEntryPoint;
    fclose(fp);
}

static void inject_code()
{
    unsigned char buf[ENTRY_CODE_SIZE + ENTRY_CODE_DATA_SIZE];
    unsigned char *pcode = buf, *pdata = buf + ENTRY_CODE_SIZE;
    unsigned char jmpinstr[5];
    struct entry_code_data data;
    assert(sizeof(entry_code) == ENTRY_CODE_SIZE);
    assert(sizeof(data) == ENTRY_CODE_DATA_SIZE);
    assert(strlen(DLL_NAME) < sizeof(data.dllname));
    assert(strlen(DLL_FUNC) < sizeof(data.dllfunc));

    memset(buf, 0, sizeof(buf));
    memset(&data, 0, sizeof(data));
    
    unsigned remote_addr = malloc_remote(sizeof(buf));

    data.entry_addr = pe_entry;
    data.entry_orig_datasize = 5;
    memcpy_from_remote(data.entry_orig_data, data.entry_addr, 5);
    data.GetCurrentProcess = (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetCurrentProcess");
    data.WriteProcessMemory = (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "WriteProcessMemory");
    data.LoadLibrary = (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    data.GetProcAddress = (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProcAddress");
    strcpy(data.dllname, DLL_NAME);
    strcpy(data.dllfunc, DLL_FUNC);
    memcpy(pdata, &data, sizeof(data));
    memcpy(pcode, entry_code, sizeof(entry_code));
    unsigned dataimm = remote_addr + ENTRY_CODE_SIZE;
    memcpy(pcode + ENTRY_CODE_DATA_OFFSET, &dataimm, 4);
    memcpy_to_remote(remote_addr, buf, sizeof(buf));
    
    
    jmpinstr[0] = 0xE9;
    unsigned jmpimm = remote_addr - (pe_entry + 5);
    memcpy(jmpinstr + 1, &jmpimm, 4);
    memcpy_to_remote(pe_entry, jmpinstr, 5);
}


static void create_process()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    BOOL ret = CreateProcess(EXE_NAME, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!ret) fail("CreateProcess() failed.");
    rprocess = pi.hProcess;
    rthread = pi.hThread;
}



int main()
{
    find_pe_entry();
    create_process();
    inject_code();
    start_execute();
    clean_up();
    return 0;
}
