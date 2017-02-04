// fix dumped file for PAL3.exe
// see notes20160712.txt for details

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"

#define MAXIMPORTADDRCOUNT 10


// token reader with unread feature
// token means string without space chars
static FILE *tokenfp;
static char token_buffer[MAXLINE];
static int token_buffer_unread_flag;
static void tokenreader_init(const char *filename)
{
    tokenfp = fopen(filename, "r");
    assert(tokenfp);
    token_buffer_unread_flag = 0;
}
static void tokenreader_cleanup()
{
    fclose(tokenfp);
}
static char *read_token()
{
    if (token_buffer_unread_flag) {
        token_buffer_unread_flag = 0;
    } else {
        int ret = fscanf(tokenfp, MAXLINEFMT, token_buffer);
        assert(ret == 1);
    }
    return token_buffer;
}
static void unread_token()
{
    // a token is still valid after unread_token(token) is called
    assert(token_buffer_unread_flag == 0); // at most one token can be unread
    token_buffer_unread_flag = 1;
}







static unsigned load_section(void *buf, unsigned bufsize, const char *sname, int fillbyte)
{
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), "dump%s", sname);
    FILE *fp = fopen(fn, "rb");
    assert(fp);
    memset(buf, fillbyte, bufsize);
    unsigned ret = fread(buf, 1, bufsize, fp);
    assert(ret < bufsize); // ensure buffer not full
    fclose(fp);
    return ret;
}

static void save_section(void *buf, unsigned size, const char *sname)
{
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), "dump%s.fixed", sname);
    FILE *fp = fopen(fn, "wb");
    unsigned ret = fwrite(buf, 1, size, fp);
    assert(ret == size);
    fclose(fp);
}

// 16MB should enough
#define BUFSIZE (1048576 * 16)
static unsigned stextsize, oldstextsize, stextsizelimit;
static char stext[BUFSIZE];
static unsigned srdatasize, oldsrdatasize, srdatasizelimit;
static char srdata[BUFSIZE];


#define ROUND_UP_INC(value, graduate) ((value) - (value) % (graduate) + (graduate))
#define ROUND_UP(value, graduate) ((value) % (graduate) == 0 ? (value) : ROUND_UP_INC(value, graduate))


#define INT3 0xCC
#define NOP 0x90

// MANUALLY enter section information here
#define S_TEXT_BASE 0x00401000
#define S_RDATA_BASE 0x0056A000
#define IAT_START 0x56A000
#define IAT_END (0x56A000 + 0x1000)

#define IS_GOOD_IATADDR(x) (IAT_START <= (x) && (x) < IAT_END)

// what byte do you want to fill the remaining space
#define S_TEXT_FILLBYTE INT3
#define S_RDATA_FILLBYTE 0x00

static unsigned search_and_fix_imm(unsigned vaddr, unsigned char *data, unsigned psize, void *magic, unsigned magiclen, unsigned oldimm, unsigned newimm)
{
    unsigned i;
    unsigned fixcount = 0;
    for (i = magiclen; i < psize - 4; i++) {
        unsigned imm;
        memcpy(&imm, &data[i], sizeof(imm));
        if (imm == oldimm) {
            assert(memcmp(&data[i - magiclen], magic, magiclen) == 0);
            memcpy(&data[i], &newimm, sizeof(newimm));
            printf("  IMMFIX: OFFSET=%08X OLDIMM=%08X NEWIMM=%08X\n", vaddr + i, oldimm, newimm);
            fixcount++;
        }
    }
    return fixcount;
}
static void search_and_fix_jmp(unsigned vaddr, unsigned char *data, unsigned psize, unsigned jtarget, unsigned new_jtarget)
{
    // search JMP/JCC/CALL to 'jtarget' in range [vaddr, psize)
    unsigned i;
    unsigned fixcount = 0;
    //    OP AA BB CC DD ??
    //       ^[vaddr+i]  ^[vaddr+i+4]
    for (i = 1; i < psize - 4; i++) {
        unsigned imm, cur_jtarget;
        memcpy(&imm, &data[i], sizeof(imm));
        cur_jtarget = vaddr + i + 4 + imm;
        if (cur_jtarget == jtarget) {
            unsigned op1 = data[i - 1], op0 = i >= 2 ? data[i - 2] : 0;
            assert(op1 == 0xE9 || op1 == 0xE8 || (op0 == 0x0F && (op1 & 0xF0) == 0x80));
            printf("  JMPFIX: VADDR=%08X OP=%02X IMM=%08X => JTARGET=%08X", vaddr + i - 1, op1, imm, jtarget);
            imm = new_jtarget - (vaddr + i + 4);
            printf(" NEWIMM=%08X\n", imm);
            memcpy(&data[i], &imm, sizeof(imm));
            fixcount++;
        }
    }
    assert(fixcount == 1);
}

static int try_fill(unsigned vaddr, unsigned len)
{
    // should be unsigned compare, no need to check low limit
    if (vaddr - S_TEXT_BASE < stextsize - len) {
        memset(stext + (vaddr - S_TEXT_BASE), S_TEXT_FILLBYTE, len);
        printf("  FILL: IN .text  ADDR=%08X LEN=%08X BYTE=%02X\n", vaddr, len, S_TEXT_FILLBYTE);
        return 1;
    } else if (vaddr - S_RDATA_BASE < srdatasize - len) {
        memset(srdata + (vaddr - S_RDATA_BASE), S_RDATA_FILLBYTE, len);
        printf("  FILL: IN .rdata ADDR=%08X LEN=%08X BYTE=%02X\n", vaddr, len, S_RDATA_FILLBYTE);
        return 1;
    } else return 0;
}
static void fix_jmp()
{
    unsigned addr, dest;
    char *s = read_token(); assert(strcmp(s, "JMP") == 0);
    s = read_token(); assert(strcmp(s, "ADDR") == 0);
    sscanf(read_token(), "%x", &addr);
    s = read_token(); assert(strcmp(s, "DEST") == 0);
    sscanf(read_token(), "%x", &dest);
    printf("FIX: JMP ADDR=%08X DEST=%08X\n", addr, dest);
    search_and_fix_jmp(S_TEXT_BASE, stext, stextsize, addr, dest);
    try_fill(addr, 5);
}

static void fix_func()
{
    unsigned i;
    unsigned len;
    unsigned funcid;
    unsigned rfuncaddr; // function addr related to stext[]
    unsigned funcaddr; // read function address when executing
    unsigned funcjmp;
    char *s = read_token(); assert(strcmp(s, "FUNC") == 0);
    sscanf(read_token(), "%x", &funcid);
    s = read_token(); assert(strcmp(s, "DATA") == 0);
    s = read_token(); assert(strcmp(s, "LEN") == 0);
    sscanf(read_token(), "%x", &len);
    
    // append data to end of .text
    stextsize = ROUND_UP_INC(stextsize, 0x10);
    rfuncaddr = stextsize;
    funcaddr = stextsize + S_TEXT_BASE;
    printf("FUNC %08X => %08X\n", funcid, funcaddr);
    
    // copy data
    for (i = 0; i < len; i++) {
        unsigned b;
        sscanf(read_token(), "%x", &b);
        stext[stextsize + i] = b;
    }
    stextsize += len;
    
    while (strcmp(read_token(), "RELOCATE") == 0) {
        unsigned offset, dest;
        s = read_token(); assert(strcmp(s, "OFFSET") == 0);
        sscanf(read_token(), "%x", &offset);
        s = read_token(); assert(strcmp(s, "DEST") == 0);
        sscanf(read_token(), "%x", &dest);
        
        unsigned imm;
        imm = dest - (funcaddr + offset + 4);
        memcpy(&stext[rfuncaddr + offset], &imm, sizeof(imm));
        printf("  FUNC RELOCATE: OFFSET=%08X NEWIMM=%08X\n", offset, imm);
    }
    unread_token();
    
    
    s = read_token(); assert(strcmp(s, "FUNCJMP") == 0);
    s = read_token(); assert(strcmp(s, "ADDR") == 0);
    sscanf(read_token(), "%x", &funcjmp);
    search_and_fix_jmp(S_TEXT_BASE, stext, stextsize, funcjmp, funcaddr);
    
    // fill junk instrs with S_TEXT_FILLBYTE
    unsigned junksize;
    switch (funcid) { // MANUALLY enter the size of junk here
        case 0x59E0: junksize = 0x0E0; break;
        case 0x57E0: junksize = 0x1F0; break;
        case 0x55A0: junksize = 0x1B0; break;
        case 0x5AC0: junksize = 0x0E0; break;
        default: assert(0);
    }
    
    int ret = try_fill(funcjmp, junksize);
    assert(ret);
    
    s = read_token(); assert(strcmp(s, "ENDFUNC") == 0);
    assert(stextsize <= stextsizelimit);
}
static void fix_library()
{
    char *s = read_token(); assert(strcmp(s, "LIBRARY") == 0);
    char dllname[MAXLINE];
    strncpy(dllname, read_token(), sizeof(dllname));
    dllname[sizeof(dllname) - 1] = '\0';
    
    printf("LIBRARY: %s\n", dllname);
    while (1) {
        s = read_token();
        if (strcmp(s, "IMPORT") == 0) {
            char funcname[MAXLINE];
            strncpy(funcname, read_token(), sizeof(funcname));
            funcname[sizeof(funcname) - 1] = '\0';
            
            s = read_token(); assert(strcmp(s, "ADDR") == 0);
            
            unsigned ptraddr[MAXIMPORTADDRCOUNT];
            int cnt = 0;
            while (strcmp((s = read_token()), "END")) {
                assert(cnt < MAXIMPORTADDRCOUNT);
                sscanf(s, "%x", &ptraddr[cnt++]);
            }
            int i;
            unsigned goodaddr = 0;
            for (i = 0; i < cnt; i++) {
                if (IS_GOOD_IATADDR(ptraddr[i])) {
                    assert(goodaddr == 0);
                    goodaddr = ptraddr[i];
                }
            }
            assert(goodaddr != 0);
            for (i = 0; i < cnt; i++) {
                if (ptraddr[i] != goodaddr) {
                    printf(" FIX  %08X => %08X  FUNC %s\n", ptraddr[i], goodaddr, funcname);
                    unsigned fixcnt;
                    fixcnt = search_and_fix_imm(S_TEXT_BASE, stext, stextsize, "\xFF\x15", 2, ptraddr[i], goodaddr);
                    assert(fixcnt == 1);
                    try_fill(ptraddr[i], 4);
                }
            }
        } else if (strcmp(s, "ENDLIBRARY") == 0) {
            return;
        } else assert(0);
    }
    while (strcmp(read_token(), "ENDLIBRARY"));
}

static void fix_manually()
{   
    // MANUALLY fix something here
    /* no need to alloc memory here, see notes20160713.txt
    // fix VirtualAlloc memblock
    unsigned vallocmemptr = 0x1896D20; // place the 0x80 memblock at end of .data
    srdatasize = ROUND_UP_INC(srdatasize, 0x10);
    unsigned vallocmemptrptr = srdatasize + S_RDATA_BASE;
    memcpy(&srdata[srdatasize], &vallocmemptr, sizeof(vallocmemptr));
    srdatasize += 4;
    
    // fix reference to the pointer to pointer to VirtualAlloc memblock
    unsigned ret;
    ret = search_and_fix_imm(S_TEXT_BASE, stext, stextsize, 0x1895300, vallocmemptrptr);
    assert(ret == 3);
    */
    
    // patch out VirtualAlloc() related mem area instructions
    memset(stext + 0x140a73, NOP, 0x15); // [541a73, 541a88)
    memset(stext + 0x140c10, NOP, 0x12); // [541c10, 541c22)
    
    // patch another range of junk
    memset(stext + 0x6a50, S_TEXT_FILLBYTE, 0x60); // [407a50, 407ab0)
}

int main()
{
    oldstextsize = stextsize = load_section(stext, sizeof(stext), ".text", S_TEXT_FILLBYTE);
    stextsizelimit = ROUND_UP(stextsize, 0x1000); // round to page
    oldsrdatasize = srdatasize = load_section(srdata, sizeof(srdata), ".rdata", S_RDATA_FILLBYTE);
    srdatasizelimit = ROUND_UP(srdatasize, 0x1000); // round to page
    
    tokenreader_init("analysis.txt");
    while (1) {
        char *s = read_token();
        unread_token();
        if (strcmp(s, "END") == 0) break;
        else if (strcmp(s, "JMP") == 0) fix_jmp();
        else if (strcmp(s, "FUNC") == 0) fix_func();
        else if (strcmp(s, "LIBRARY") == 0) fix_library();
        else {
            printf("unknown token %s\n", s);
            assert(0);
        }
    }
    tokenreader_cleanup();
    
    fix_manually();
    
    assert(stextsize <= stextsizelimit);
    assert(srdatasize <= srdatasizelimit);
    
    printf("\n\n===== SUMMARY =====\n\n");
    printf(".text   BASE=%08X OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", S_TEXT_BASE, oldstextsize, stextsize, stextsizelimit);
    printf(".rdata  BASE=%08X OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", S_RDATA_BASE, oldsrdatasize, srdatasize, srdatasizelimit);
    
    save_section(stext, stextsize, ".text");
    save_section(srdata, srdatasize, ".rdata");
    return 0;
}
