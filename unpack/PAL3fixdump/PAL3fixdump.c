// fix dumped file for PAL3.exe
// see notes20160712.txt for details

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%" TO_STR(MAXLINE) "s"


// token reader with unread feature
// token means string without space chars
static FILE *tokenfp;
static char token_buffer[MAXLINE];
static int token_buffer_unread_flag;
static void tokenreader_init(const char *filename)
{
    tokenfp = fopen(filename, "r");
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





// what byte do you want to fill the remaining space
#define FILLBYTE 0xCC

static unsigned load_section(void *buf, unsigned bufsize, const char *sname)
{
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), "dump%s", sname);
    FILE *fp = fopen(fn, "rb");
    memset(buf, FILLBYTE, bufsize);
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


#define ROUND_UP(value, graduate) ((value) % (graduate) == 0 ? (value) : ((value) - (value) % (graduate) + (graduate)))


// MANUALLY enter section information here
#define S_TEXT_BASE 0x00401000



static unsigned search_and_fix_jmp(unsigned vaddr, unsigned char *data, unsigned psize, unsigned jtarget, unsigned new_jtarget)
{
    // search JMP/JCC/CALL to 'jtarget' in range [vaddr, psize)
    unsigned i;
    unsigned ret = -1;
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
            ret = i;
        }
    }
    assert(fixcount == 1);
    return ret; // return value points to imm
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
    if (addr - S_TEXT_BASE < stextsize - 5) {
        memset(stext + (addr - S_TEXT_BASE), FILLBYTE, 5);
        printf("  FILL: ADDR=%08X BYTE=%02X\n", addr, FILLBYTE);
    }
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
    stextsize = ROUND_UP(stextsize, 0x10);
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
    
    // fill junk instrs with FILLBYTE
    unsigned junksize;
    switch (funcid) { // MANUALLY enter the size of junk here
        case 0x59E0: junksize = 0x0E0; break;
        case 0x57E0: junksize = 0x1F0; break;
        case 0x55A0: junksize = 0x1B0; break;
        case 0x5AC0: junksize = 0x0E0; break;
        default: assert(0);
    }
    memset(stext + (funcjmp - S_TEXT_BASE), FILLBYTE, junksize);
    printf("  FILL: ADDR=%08X LEN=%08X BYTE=%02X\n", funcjmp, junksize, FILLBYTE);
    
    s = read_token(); assert(strcmp(s, "ENDFUNC") == 0);
    assert(stextsize <= stextsizelimit);
}
static void fix_library()
{
    char *s = read_token(); assert(strcmp(s, "LIBRARY") == 0);
    printf("LIBRARY ingored!\n");
    while (strcmp(read_token(), "ENDLIBRARY"));
}

int main()
{
    oldstextsize = stextsize = load_section(stext, sizeof(stext), ".text");
    stextsizelimit = ROUND_UP(stextsize, 0x1000); // round to page
    
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
    
    assert(stextsize <= stextsizelimit);
    
    printf("\n\n===== SUMMARY =====\n\n");
    printf(".text OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", oldstextsize, stextsize, stextsizelimit);
    
    save_section(stext, stextsize, ".text");
    return 0;
}
