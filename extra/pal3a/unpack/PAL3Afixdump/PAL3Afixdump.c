// fix dumped file for PAL3.exe
// see notes20160712.txt for details

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define TOPTR(addr) ((void *)(addr))

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"

#define MAXIMPORTADDRCOUNT 10

#define PAGE_SIZE 4096


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
static long token_tell()
{
    assert(token_buffer_unread_flag == 0);
    return ftell(tokenfp);
}
static void token_seek(long p)
{
    assert(token_buffer_unread_flag == 0);
    fseek(tokenfp, p, SEEK_SET);
}







static unsigned load_section(const char *fmt, void *buf, unsigned bufsize, const char *sname, int fillbyte)
{
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), fmt, sname);
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

static int pass;

// 128MB should enough
#define BUFSIZE (1048576 * 128)
static unsigned stextsize, oldstextsize, stextsizelimit;
static char stext[BUFSIZE];
static char stext_finalcopy[BUFSIZE], stext_finalcopy_mask[BUFSIZE];
static unsigned srdatasize, oldsrdatasize, srdatasizelimit;
static char srdata[BUFSIZE];

// MANUALLY enter extend section information here
#define S_DATA_BASE 0x00570000
#define S_RSRC_BASE 0x02294000
static unsigned sdatasize, oldsdatasize, sdatasizelimit = (S_RSRC_BASE - S_DATA_BASE);
static char sdata[BUFSIZE];

#define S_EXTTEXT_BASE 0x0229A000
#define S_EXTRDATA_BASE (S_EXTTEXT_BASE + 0x1000)
#define S_EXTDATA_BASE (S_EXTRDATA_BASE + 0x4000)

static unsigned sexttextsize, sexttextsizelimit = (S_EXTDATA_BASE - S_EXTRDATA_BASE);
static unsigned sexttextsize_pass1;
static char sexttext[BUFSIZE];
static unsigned sextdatasize;

#define ROUND_UP_INC(value, graduate) ((value) - (value) % (graduate) + (graduate))
#define ROUND_UP(value, graduate) ((value) % (graduate) == 0 ? (value) : ROUND_UP_INC(value, graduate))


#define INT3 0xCC
#define NOP 0x90

// MANUALLY enter section information here
#define S_TEXT_BASE 0x00401000
#define S_RDATA_BASE 0x00558000
#define IAT_START 0x00558000
#define IAT_END 0x0055860C

#define IS_GOOD_IATADDR(x) (IAT_START <= (x) && (x) < IAT_END)

// what byte do you want to fill the remaining space
#define S_TEXT_FILLBYTE INT3
#define S_RDATA_FILLBYTE 0x00
#define S_DATA_FILLBYTE 0x00

static unsigned search_and_fix_iat(unsigned vaddr, unsigned char *data, unsigned psize, unsigned oldimm, unsigned newimm)
{
    unsigned i;
    unsigned fixcount = 0;
    unsigned magiclen = 2;
    void *magic1 = "\xFF\x15";
    void *magic2 = "\xFF\x35";

    for (i = magiclen; i < psize - 4; i++) {
        unsigned imm;
        memcpy(&imm, &data[i], sizeof(imm));
        if (imm == oldimm) {
            printf("  IMMFIX: OFFSET=%08X OLDIMM=%08X NEWIMM=%08X\n", vaddr + i, oldimm, newimm);
            assert(memcmp(&data[i - magiclen], magic1, magiclen) == 0 || memcmp(&data[i - magiclen], magic2, magiclen) == 0);
            memcpy(&data[i], &newimm, sizeof(newimm));
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
        if (imm == jtarget) {
            printf("  WARNING: IMMADDR=%08X\n", i);
            assert(0);
        }
        cur_jtarget = vaddr + i + 4 + imm;
        if (cur_jtarget == jtarget) {
            unsigned instr_vaddr = vaddr + i - 1;
            unsigned op1 = data[i - 1], op0 = i >= 2 ? data[i - 2] : 0;
            printf("  JMPFIX: VADDR=%08X OP=%02X IMM=%08X => JTARGET=%08X", instr_vaddr, op1, imm, jtarget);
            imm = new_jtarget - (vaddr + i + 4);
            printf(" NEWIMM=%08X\n", imm);
            
            int flag = 1;
            
            // MANUALLY enter exceptions here
            switch (instr_vaddr) {
                if (pass == 1) {
            case 0x004E52C3:
            case 0x0051C67A:            
            case 0x0040A84B:
                    printf(" WARNING: IGNORE INSTR AT %08X\n", instr_vaddr);
                    assert(!(op1 == 0xE9 || op1 == 0xE8 || (op0 == 0x0F && (op1 & 0xF0) == 0x80)));
                    flag = 0;
                    break;
                }
            default:
                assert(op1 == 0xE9 || op1 == 0xE8 || (op0 == 0x0F && (op1 & 0xF0) == 0x80));
            }

            if (flag) {
                memcpy(&data[i], &imm, sizeof(imm));
                fixcount++;
            }
        }
    }
    
    assert(fixcount <= 1);
    // MANUALLY enter exceptions here
    switch (jtarget) {
    case 0x0042E73B:
    case 0x00526283:
    case 0x005263EC:
    case 0x00526746:
    case 0x0229D23A:
        if (pass == 1) {
            printf(" WARNING: NO FIX FOR %08X\n", jtarget);
            assert(fixcount == 0);
            break;
        }
    default:
        assert(fixcount == 1);
    }
}

static int try_fill(unsigned vaddr, unsigned len)
{
    // should be unsigned compare, no need to check low limit
    if (vaddr - S_TEXT_BASE < stextsize - len) {
        printf("  FILL: IN .text  ADDR=%08X LEN=%08X BYTE=%02X\n", vaddr, len, S_TEXT_FILLBYTE);
        memset(stext + (vaddr - S_TEXT_BASE), S_TEXT_FILLBYTE, len);
        return 1;
    } else if (vaddr - S_RDATA_BASE < srdatasize - len) {
        printf("  FILL: IN .rdata ADDR=%08X LEN=%08X BYTE=%02X\n", vaddr, len, S_RDATA_FILLBYTE);
        memset(srdata + (vaddr - S_RDATA_BASE), S_RDATA_FILLBYTE, len);
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
    unsigned rfuncaddr; // relative function address to segment start
    unsigned funcaddr; // real function address when executing
    unsigned funcjmp, funcjmp2;
    char *s = read_token(); assert(strcmp(s, "FUNC") == 0);
    sscanf(read_token(), "%x", &funcid);
    
    // read space information
    unsigned dll_space, exe_space;
    s = read_token(); assert(strcmp(s, "SPACE") == 0);
    s = read_token(); assert(strcmp(s, "DLL") == 0);
    sscanf(read_token(), "%x", &dll_space);
    s = read_token(); assert(strcmp(s, "EXE") == 0);
    sscanf(read_token(), "%x", &exe_space);
    s = read_token(); assert(strcmp(s, "DIFF") == 0);
    s = read_token();
    printf("FUNC %08X\n  DLLSPACE %08X EXESPACE %08X SPACEDIFF %s\n", funcid, dll_space, exe_space, s);
    
    // read function jump information
    long tp = token_tell();
    while (strcmp(read_token(), "FUNCJMP"));
    s = read_token(); assert(strcmp(s, "ADDR") == 0);
    sscanf(read_token(), "%x", &funcjmp);
    token_seek(tp);
    printf("  FUNCJMP %08X\n", funcjmp);
    
    // read function data information
    s = read_token(); assert(strcmp(s, "DATA") == 0);
    s = read_token(); assert(strcmp(s, "LEN") == 0);
    sscanf(read_token(), "%x", &len);
    
    // copy data
    static char cbuf[BUFSIZE];
    for (i = 0; i < len; i++) {
        unsigned b;
        sscanf(read_token(), "%x", &b);
        cbuf[i] = b;
    }

    // determine where to place the function
    unsigned inplace_flag = (dll_space == exe_space);
    if (inplace_flag) {
        funcaddr = funcjmp;
    } else {
        sexttextsize = ROUND_UP(sexttextsize, 0x10);
        funcaddr = S_EXTTEXT_BASE + sexttextsize;
    }
    printf("  FUNCADDR %08X => %08X\n", funcid, funcaddr);
    
    // do reloactions
    while (strcmp(read_token(), "PE_RELOC") == 0) {
        unsigned offset, pe_base;
        s = read_token(); assert(strcmp(s, "OFFSET") == 0);
        sscanf(read_token(), "%x", &offset);
        s = read_token(); assert(strcmp(s, "PE_BASE") == 0);
        sscanf(read_token(), "%x", &pe_base);
        
        // old: pe_base + funcid + someoffset
        // new: funcaddr + someoffset
        unsigned oldval, newval, someoffset;
        memcpy(&oldval, &cbuf[offset], sizeof(oldval));
        someoffset = oldval - (pe_base + funcid);
        assert(someoffset + 4 < len);
        newval = someoffset + funcaddr;
        memcpy(&cbuf[offset], &newval, sizeof(newval));
        printf("  PE_RELOC: OFFSET=%08X OLD=%08X NEW=%08X\n", offset, oldval, newval);
    }
    unread_token();
    while (strcmp(read_token(), "RELOCATE") == 0) {
        unsigned offset, dest;
        s = read_token(); assert(strcmp(s, "OFFSET") == 0);
        sscanf(read_token(), "%x", &offset);
        s = read_token(); assert(strcmp(s, "DEST") == 0);
        sscanf(read_token(), "%x", &dest);
        
        unsigned imm;
        imm = dest - (funcaddr + offset + 4);
        memcpy(&cbuf[offset], &imm, sizeof(imm));
        printf("  FUNC RELOCATE: OFFSET=%08X NEWIMM=%08X\n", offset, imm);
    }
    unread_token();
    
    
    // fix reference to funcjmp if necessary
    s = read_token(); assert(strcmp(s, "FUNCJMP") == 0);
    s = read_token(); assert(strcmp(s, "ADDR") == 0);
    sscanf(read_token(), "%x", &funcjmp2);
    assert(funcjmp == funcjmp2);
    if (!inplace_flag) {
        search_and_fix_jmp(S_TEXT_BASE, stext, stextsize, funcjmp, funcaddr);
    }
    
    // fill junk instrs with S_TEXT_FILLBYTE if needed
    if (!inplace_flag) {
        int ret = try_fill(funcjmp, exe_space);
        assert(ret);
    }
    
    // copy data
    if (inplace_flag) {
        rfuncaddr = (funcaddr - S_TEXT_BASE);
        // delay copy to avoid some other function touch our code
        memcpy(&stext_finalcopy[rfuncaddr], cbuf, len);
        memset(&stext_finalcopy_mask[rfuncaddr], -1, len);
    } else {
        rfuncaddr = (funcaddr - S_EXTTEXT_BASE);
        assert(rfuncaddr == sexttextsize);
        sexttextsize += len;
        memcpy(&sexttext[rfuncaddr], cbuf, len);
    }
    
    s = read_token(); assert(strcmp(s, "ENDFUNC") == 0);
    assert(stextsize <= stextsizelimit);
    assert(sexttextsize <= sexttextsizelimit);
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
                    fixcnt = search_and_fix_iat(S_TEXT_BASE, stext, stextsize, ptraddr[i], goodaddr);
                    // MANUALLY enter exceptions here
                    switch (ptraddr[i]) {
                        if (pass == 1) {
                    case 0x022A7D88:
                    case 0x0041D930:
                    case 0x00526268:
                    case 0x005266E4:
                    case 0x0056BE8C:
                            printf("  WARNING: NO FIX FOR %08X\n", ptraddr[i]);
                            assert(fixcnt == 0);
                            break;
                        }
                    default:
                        assert(fixcnt == 1);
                    }
                    
                    try_fill(ptraddr[i], 4);
                }
            }
        } else if (strcmp(s, "ENDLIBRARY") == 0) {
            return;
        } else assert(0);
    }
    while (strcmp(read_token(), "ENDLIBRARY"));
}

#define MAXVI 10000
static struct vi {
    unsigned ptr;
    unsigned size;
} vi[MAXVI];
static int nr_vi;
static int vi_cmp(const void *a, const void *b)
{
    const struct vi *va = a, *vb = b;
    if (va->ptr < vb->ptr) return -1;
    if (va->ptr > vb->ptr) return 1;
    return 0;
}

static void fix_valloc()
{
    FILE *vifp = fopen("vallocinfo.txt", "r");
    nr_vi = 0;
    while (fscanf(vifp, " VALLOC PTR %x SIZE %x", &vi[nr_vi].ptr, &vi[nr_vi].size) == 2) {
        nr_vi++;
    }
    fclose(vifp);
    qsort(vi, nr_vi, sizeof(struct vi), vi_cmp);
    int i;
    for (i = 0; i < nr_vi; i++) {
        // alloc space
        sextdatasize = ROUND_UP(sextdatasize, 0x1000);
        unsigned page_vaddr = sextdatasize + S_EXTDATA_BASE;
        sextdatasize += vi[i].size;
        
        // make ptr
        unsigned raddr = vi[i].ptr - S_DATA_BASE;
        if (raddr + 4 > sdatasize) sdatasize = raddr + 4;
        memcpy(&sdata[raddr], &page_vaddr, sizeof(page_vaddr));
        
        printf("VALLOC PTR %08X SIZE %08X => %08X\n", vi[i].ptr, vi[i].size, page_vaddr);
    }
    
    // MANUALLY fill nop
    memset(&stext[0x52bafc - S_TEXT_BASE], NOP, 5);
}

static void load_stext_finalcopy()
{
    printf("LOAD FINALCOPY DATA\n");
    unsigned r;
    r = load_section("dump%s.fixed", stext_finalcopy, sizeof(stext_finalcopy), ".text.finalcopy", S_TEXT_FILLBYTE);
    assert(stextsize == r);
    r = load_section("dump%s.fixed", stext_finalcopy_mask, sizeof(stext_finalcopy_mask), ".text.finalcopy.mask", 0x00);
    assert(stextsize == r);
}
static void save_stext_finalcopy()
{
    printf("SAVE FINALCOPY DATA\n");
    save_section(stext_finalcopy, stextsize, ".text.finalcopy");
    save_section(stext_finalcopy_mask, stextsize, ".text.finalcopy.mask");
}

static void do_stext_finalcopy()
{
    unsigned i;
    unsigned cnt = 0;
    for (i = 0; i < stextsize; i++) {
        if (stext_finalcopy_mask[i]) {
            stext[i] = stext_finalcopy[i];
            cnt++;
        }
    }
    printf("FINAL COPY %08X BYTES\n", cnt);
    if (pass == 1) save_stext_finalcopy();
}

int main(int argc, char *argv[])
{
    assert(argc == 2);
    pass = atoi(argv[1]);
    assert(pass == 1 || pass == 2);
    
    memset(sexttext, INT3, sizeof(sexttext));
    
    oldstextsize = stextsize = load_section("dump%s", stext, sizeof(stext), ".text", S_TEXT_FILLBYTE);
    stextsizelimit = ROUND_UP(stextsize, 0x1000); // round to page
    oldsrdatasize = srdatasize = load_section("dump%s", srdata, sizeof(srdata), ".rdata", S_RDATA_FILLBYTE);
    srdatasizelimit = ROUND_UP(srdatasize, 0x1000); // round to page
    
    oldsdatasize = sdatasize = load_section("dump%s", sdata, sizeof(sdata), ".data", S_DATA_FILLBYTE);
    
    if (pass == 2) {
        // load-apply-reset last finalcopy data
        load_stext_finalcopy();
        do_stext_finalcopy();
        memset(stext_finalcopy, 0, sizeof(stext_finalcopy));
        memset(stext_finalcopy_mask, 0, sizeof(stext_finalcopy_mask));
        
        // load previously generated exttext data
        sexttextsize_pass1 = sexttextsize = load_section("dump%s.fixed", sexttext, sizeof(sexttext), ".exttext", S_TEXT_FILLBYTE);
        unsigned new_text_size = S_EXTTEXT_BASE + sexttextsize - S_TEXT_BASE;
        assert(new_text_size > stextsize);
        
        // update stext size
        stextsize = new_text_size;
        stextsizelimit = ROUND_UP(stextsize, 0x1000); // round to page
        
        // copy data to stext
        memcpy(&stext[S_EXTTEXT_BASE - S_TEXT_BASE], sexttext, sexttextsize);
        
        // reset sexttext
        sexttextsize = 0;
        memset(sexttext, INT3, sizeof(sexttext));
    }
    
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
    
    fix_valloc();
    
    do_stext_finalcopy();
    
    if (pass == 2) {
        // copy data back to sexttext
        memcpy(sexttext, &stext[S_EXTTEXT_BASE - S_TEXT_BASE], sexttextsize);
        assert(sexttextsize == sexttextsize_pass1);
        
        // restore stext size
        stextsize = oldstextsize;
    }

    assert(stextsize <= stextsizelimit);
    assert(srdatasize <= srdatasizelimit);
    assert(sdatasize <= sdatasizelimit);
    assert(sdatasize >= oldsdatasize);
    assert(sexttextsize <= sexttextsizelimit);
    
    printf("\n\n===== SUMMARY =====\n\n");
    printf(".text    BASE=%08X OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", S_TEXT_BASE, oldstextsize, stextsize, stextsizelimit);
    printf(".rdata   BASE=%08X OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", S_RDATA_BASE, oldsrdatasize, srdatasize, srdatasizelimit);
    printf(".data    BASE=%08X OLDSIZE=%08X NEWSIZE=%08X LIMIT=%08X\n", S_DATA_BASE, oldsdatasize, sdatasize, sdatasizelimit);
    printf(".exttext BASE=%08X NEWSIZE=%08X\n", S_EXTTEXT_BASE, sexttextsize);
    printf(".extdata BASE=%08X NEWSIZE=%08X\n", S_EXTDATA_BASE, sextdatasize);
    
    save_section(stext, stextsize, ".text");
    save_section(srdata, srdatasize, ".rdata");
    save_section(sdata, sdatasize, ".data");
    save_section(sexttext, sexttextsize, ".exttext");
    
    if (pass == 1) {
        printf("\n\nNOTE: PLEASE RUN PASS 2\n");
    }
    return 0;
}
