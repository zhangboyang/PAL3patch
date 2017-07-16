// analyse the unpacker PAL3.DLL
// see notes20160712.txt for details

#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#define EXEBASE 0x00400000
#define MAXLINE 4096

static void *base_addr;
static FILE *fp;

static void analyse_jmp()
{
    /*
    sample code fragment for making a in-module JMP
    EBP = 00400000 (base of PAL3.EXE)
    01991241   8D8D 30B21300       LEA ECX,[EBP+13B230]
    01991247   8D85 547A0000       LEA EAX,[EBP+7A54]
    0199124D   8D50 05             LEA EDX,[EAX+5]
    01991250   29D1                SUB ECX,EDX
    01991252   C600 E9             MOV BYTE PTR DS:[EAX],0E9
    01991255   8948 01             MOV DWORD PTR DS:[EAX+1],ECX
    
    result:
    00407A54   E9 D7371300         JMP 0053B230
    */
    
    const char *pattern =
        "\x8D\x8D\x00\x00\x00\x00"
        "\x8D\x85\x00\x00\x00\x00"
        "\x8D\x50\x05"
        "\x29\xD1"
        "\xC6\x00\xE9"
        "\x89\x48\x01";
    void *ptr;
    // MANUALLY enter 'begin' and 'end' here
    void *begin = base_addr + 0x1241; // 01991241 => base + 0x1241; BEGIN
    void *end = base_addr + 0x14AE; // 019914AE => base + 0x14AE; END
    for (ptr = begin; ptr != end; ptr += 0x17) {
        char buf[0x17];
        memcpy(buf, ptr, sizeof(buf));
        unsigned addr, dest;
        memcpy(&dest, &buf[2], sizeof(dest));
        memcpy(&addr, &buf[8], sizeof(addr));
        memset(&buf[2], 0, 4);
        memset(&buf[8], 0, 4);
        //printf("PTR = %p\n", ptr);
        assert(memcmp(buf, pattern, 0x17) == 0);
        addr += EXEBASE;
        dest += EXEBASE;
        fprintf(fp, "JMP  ADDR %08X  DEST %08X\n", addr, dest);
    }
}

static void analyse_func()
{
    /*
    sample code fragment for making a func
    EBX = 01990000 (base of PAL3.DLL)
    EBP = 00400000 (base of PAL3.EXE)
    019914CA   .  B9 E0579901         MOV ECX,019957E0
    019914CF   .  2959 10             SUB DWORD PTR DS:[ECX+10],EBX
    ......
    019914F0   .  2999 AA000000       SUB DWORD PTR DS:[ECX+0AA],EBX
    ......
    01991514   .  8D85 60270200       LEA EAX,[EBP+22760]
    0199151A   .  8D50 05             LEA EDX,[EAX+5]
    0199151D   .  29D1                SUB ECX,EDX
    0199151F   .  C600 E9             MOV BYTE PTR DS:[EAX],0E9
    01991522   .  8948 01             MOV DWORD PTR DS:[EAX+1],ECX
    
    result:
    00422760    - E9 7B305701         JMP 019957E0
    */
    
    unsigned char *ptr;
    // MANUALLY enter 'begin' and 'end' here
    void *begin = base_addr + 0x14AE; // 01991241 => base + 0x1241; BEGIN
    void *end = base_addr + 0x15A8; // 019915A8 => base + 0x15A8; END
    for (ptr = begin; ptr != end; ) {
        // read function address
        unsigned funcaddr;
        assert(ptr[0] == 0xB9);
        memcpy(&funcaddr, &ptr[1], sizeof(funcaddr));
        ptr += 5;
        fprintf(fp, "FUNC %08X\n", funcaddr - (unsigned) base_addr);
        
        // read instr data
        unsigned funclen;
        switch (funcaddr - (unsigned) base_addr) {
            // MANUALLY enter func length here
            case 0x59E0: funclen = 0x0E0; break;
            case 0x57E0: funclen = 0x200; break;
            case 0x55A0: funclen = 0x1C0; break;
            case 0x5AC0: funclen = 0x0E0; break;
            default: assert(0); // unknown func
        }
        unsigned char *funcdata = (void *) funcaddr;
        unsigned i;
        fprintf(fp, "  DATA  LEN %08X", funclen);
        for (i = 0; i < funclen; i++) {
            if (i % 16 == 0) fprintf(fp, "\n    ");
            else if (i % 8 == 0) fprintf(fp, " ");
            fprintf(fp, "%02X ", funcdata[i]);
        }
        fprintf(fp, "\n");
        
        // read SUB instrs
        while (*ptr == 0x29) {
            unsigned offset;
            switch (*++ptr) {
                case 0x59: offset = (int)(signed char) *++ptr; ptr++; break; // imm is 1 byte
                case 0x99: memcpy(&offset, ++ptr, sizeof(offset)); ptr += 4; break; // imm is 4 bytes
                default: assert(0);
            }
            unsigned dest;
            memcpy(&dest, (void *)(funcaddr + offset), sizeof(dest));
            dest = funcaddr + offset + 4 + dest - (unsigned) base_addr;
            fprintf(fp, "  RELOCATE  OFFSET %08X  DEST %08X\n", offset, dest);
        }
        
        // read JMP instr address
        unsigned jmpaddr;
        assert(memcmp(ptr, "\x8D\x85", 2) == 0);
        memcpy(&jmpaddr, &ptr[2], sizeof(jmpaddr));
        jmpaddr += EXEBASE;
        ptr += 6;
        fprintf(fp, "  FUNCJMP  ADDR %08X\n", jmpaddr);
        
        // read left instrs
        assert(memcmp(ptr, "\x8D\x50\x05\x29\xD1\xC6\x00\xE9\x89\x48\x01", 11) == 0);
        ptr += 11;
        
        fprintf(fp, "ENDFUNC\n");
    }
}

static unsigned read_immediate_string(unsigned char *ptr, char *buf, unsigned bufsize)
{
    unsigned len;
    assert(*ptr++ == 0xE8);
    memcpy(&len, ptr, sizeof(len));
    ptr += 4;
    assert(len <= bufsize && len == strlen(ptr) + 1);
    memcpy(buf, ptr, len);
    return 5 + len;
}
static void check_no_space_chars(char *str)
{
    while (*str) { assert(!isspace(*str)); str++; }
}

static void analyse_library()
{
    /*
    sample code fragment for importing a func from a DLL
    ESI = GetModuleHandle()
    EDI = GetProcAddress()
    019915B4   .  E8 0D000000         CALL 019915C6
    019915B9   .  41 44 56 41 50 49 3 ASCII "ADVAPI32.dll",0
    019915C6   >  FFD6                CALL ESI
    019915C8   .  89C3                MOV EBX,EAX
        019915CA   .  E8 0C000000         CALL 019915DB
        019915CF   .  52 65 67 43 6C 6F 7 ASCII "RegCloseKey",0
        019915DB   >  53                  PUSH EBX
        019915DC   .  FFD7                CALL EDI
        019915DE   .  8985 10A01600       MOV DWORD PTR SS:[EBP+16A010],EAX
        019915DE   .  8985 10A01600       MOV DWORD PTR SS:[EBP+16A010],EAX
        ......
    ......
    */
    
    unsigned char *ptr;
    // MANUALLY enter 'begin' and 'end' here
    void *begin = base_addr + 0x15B4; // 019915B4 => base + 0x15B4; BEGIN
    void *end = base_addr + 0x5591; // 01995591 => base + 0x5591; END
    for (ptr = begin; ptr != end; ) {
        char buf[MAXLINE];
        // read DLL name
        ptr += read_immediate_string(ptr, buf, sizeof(buf));
        check_no_space_chars(buf);
        fprintf(fp, "LIBRARY %s\n", buf);

        // read some magic data        
        assert(memcmp(ptr, "\xFF\xD6\x89\xC3", 4) == 0);
        ptr += 4;

        // read func imports
        while (*ptr == 0xE8) {
            // read function name
            unsigned namelen;
            namelen = read_immediate_string(ptr, buf, sizeof(buf));
            if (ptr[namelen] != 0x53) break;
            ptr += namelen;
            check_no_space_chars(buf);
            fprintf(fp, "  IMPORT %s", buf);
            
            // read some magic data 
            assert(memcmp(ptr, "\x53\xFF\xD7", 3) == 0);
            ptr += 3;
            
            fprintf(fp, "  ADDR");
            while (memcmp(ptr, "\x89\x85", 2) == 0) {
                unsigned addr;
                memcpy(&addr, &ptr[2], sizeof(addr));
                addr += EXEBASE;
                fprintf(fp, " %08X", addr);
                ptr += 6;
            }
            fprintf(fp, " END\n");
        }
        
        fprintf(fp, "ENDLIBRARY\n");
    }
}
int main()
{
    HMODULE unpack_base;
    unpack_base = LoadLibraryEx("pal3unpack.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
    base_addr = unpack_base;
    printf("DLLBASE = %p\n", base_addr);
    assert(unpack_base);

    fp = fopen("analysis.txt", "w");
    analyse_jmp();
    analyse_func();
    analyse_library();
    fprintf(fp, "END");
    fclose(fp);

    FreeLibrary(unpack_base);
    
    printf("analyse OK!\n");
    return 0;
}
