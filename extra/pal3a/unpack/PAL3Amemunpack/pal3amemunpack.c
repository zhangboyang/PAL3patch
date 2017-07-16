// the alternate unpacker, unpack data directly in memory
// see notes20160712.txt for details

#include <windows.h>
#include <stdio.h>
#include <assert.h>

extern unsigned oep;

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"


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







#define S_EXTTEXT_BASE 0x0229A000
#define S_EXTDATA_BASE 0x0229B000
#define EXE_BEGIN 0x00401000
#define EXE_END   0x022C5000


static void load_data_from_file(unsigned char *ptr, FILE *fp)
{
    int c;
    while ((c = fgetc(fp)) != EOF) *ptr++ = c;
}

static void load_section(const char *sname, unsigned base)
{
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), "dump%s.fixed", sname); // try fixed first
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        snprintf(fn, sizeof(fn), "dump%s", sname);
        fp = fopen(fn, "rb");
    }
    assert(fp);
    load_data_from_file((void *) base, fp);
    fclose(fp);
}

static void loader()
{
    unsigned base;
    char sname[MAXLINE];
    FILE *fp = fopen("dump.summary.txt", "r");
    int i;
    for (i = 0; i < 4; i++) {
        fscanf(fp, MAXLINEFMT, sname);
        fscanf(fp, "%x%*x", &base); // 'size' is ignored!
        load_section(sname, base);
    }
    fclose(fp);
    
    load_section(".exttext", S_EXTTEXT_BASE);
}




// MANUALLY enter IAT information here
#define IAT_START 0x00558000
#define IAT_END 0x0055860C

#define IS_GOOD_IATADDR(x) (IAT_START <= (x) && (x) < IAT_END)

void link_library()
{
    char *s = read_token(); assert(strcmp(s, "LIBRARY") == 0);
    HMODULE h = GetModuleHandle(read_token());
    while (1) {
        s = read_token();
        if (strcmp(s, "IMPORT") == 0) {
            FARPROC funcaddr = GetProcAddress(h, read_token());
            s = read_token(); assert(strcmp(s, "ADDR") == 0);
            while (strcmp((s = read_token()), "END")) {
                unsigned ptraddr;
                sscanf(s, "%x", &ptraddr);
                if (IS_GOOD_IATADDR(ptraddr)) {
                    memcpy((void *)ptraddr, &funcaddr, sizeof(funcaddr));
                }
            }
        } else if (strcmp(s, "ENDLIBRARY") == 0) {
            return;
        } else assert(0);
    }
}

void dynlinker()
{
    tokenreader_init("analysis.txt");
    while (strcmp(read_token(), "LIBRARY"));
    unread_token();
    while (1) {
        char *s = read_token();
        unread_token();
        if (strcmp(s, "END") == 0) break;
        else if (strcmp(s, "LIBRARY") == 0) link_library();
        else  assert(0);
    }
    tokenreader_cleanup();
}

void unpack()
{
    oep = 0x5431C0;

    memset((void *) EXE_BEGIN, 0, EXE_END - EXE_BEGIN);
    
    loader();
    dynlinker();
    
    MessageBox(NULL, "Here we go!", "PAL3Amemunpack", 0);
}

