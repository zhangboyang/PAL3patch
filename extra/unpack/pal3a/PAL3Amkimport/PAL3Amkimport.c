// make IAT and import string table
// see notes20160713.txt for details
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"

#define BUFSIZE (16 * 1048576)
#define MAXIMPORTS 1048576
#define MAXIMPORTADDRCOUNT 10

#define ROUND_UP_INC(value, graduate) ((value) - (value) % (graduate) + (graduate))
#define ROUND_UP(value, graduate) ((value) % (graduate) == 0 ? (value) : ROUND_UP_INC(value, graduate))



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







// MANUALLY enter information here
#define IAT_START 0x00558000
#define IAT_END 0x0055860C
#define IS_GOOD_IATADDR(x) (IAT_START <= (x) && (x) < IAT_END)
#define IAT_RVA (IAT_START - 0x00400000)
#define IMPORT_RVA (0x0229A000 + 0x1000 - 0x00400000)




struct importitem {
    unsigned ptraddr;
    unsigned nameptr;
    char *funcname;
    char *dllname;
    unsigned dllid;
} ilist[MAXIMPORTS];
int ilist_cnt;

int ilist_cmp(const void *a, const void *b)
{
    unsigned aval = ((struct importitem *) a)->ptraddr;
    unsigned bval = ((struct importitem *) b)->ptraddr;
    if (aval < bval) return -1;
    if (aval > bval) return 1;
    return 0;
}

void add_import_item(unsigned ptraddr, char *funcname, char *dllname)
{
    assert(ilist_cnt < MAXIMPORTS);
    ilist[ilist_cnt].ptraddr = ptraddr;
    ilist[ilist_cnt].funcname = strdup(funcname);
    ilist[ilist_cnt].dllname = strdup(dllname);
    ilist_cnt++;
}



void read_library_data()
{
    char *s = read_token(); assert(strcmp(s, "LIBRARY") == 0);
    char dllname[MAXLINE];
    strncpy(dllname, read_token(), sizeof(dllname));
    dllname[sizeof(dllname) - 1] = '\0';
    
    printf("LIBRARY: %-18s ", dllname);
    int cnt = 0;
    while (1) {
        s = read_token();
        if (strcmp(s, "IMPORT") == 0) {
            char funcname[MAXLINE];
            strncpy(funcname, read_token(), sizeof(funcname));
            funcname[sizeof(funcname) - 1] = '\0';
            
            s = read_token(); assert(strcmp(s, "ADDR") == 0);
            int goodflag = 0;
            while (strcmp((s = read_token()), "END")) {
                unsigned ptraddr;
                sscanf(s, "%x", &ptraddr);
                if (IS_GOOD_IATADDR(ptraddr)) {
                    assert(goodflag == 0);
                    goodflag = 1;
                    add_import_item(ptraddr, funcname, dllname);
                    cnt++;
                }
            }
        } else if (strcmp(s, "ENDLIBRARY") == 0) {
            break;
        } else assert(0);
    }
    printf("IMPORT %d FUNCTIONS\n", cnt);
}

void read_data()
{
    tokenreader_init("analysis.txt");
    while (strcmp(read_token(), "LIBRARY"));
    unread_token();
    while (1) {
        char *s = read_token();
        unread_token();
        if (strcmp(s, "END") == 0) break;
        else if (strcmp(s, "LIBRARY") == 0) read_library_data();
        else assert(0);
    }
    tokenreader_cleanup();
}

unsigned short find_hint(const char *dllname, const char *funcname)
{
    // FIXME: can be optimized
    char fn[MAXLINE];
    snprintf(fn, sizeof(fn), "%s.hint.txt", dllname);
    char buf[MAXLINE];
    FILE *fp = fopen(fn, "r");
    if (!fp && strlen(dllname) > 4) {
        snprintf(fn, sizeof(fn), "%.*s.lib.hint.txt", strlen(dllname) - 4, dllname);
        fp = fopen(fn, "r");
    }
    assert(fp);
    unsigned hint = -1;
    while (fscanf(fp, "%x" MAXLINEFMT, &hint, buf) == 2) {
        if (strcmp(buf, funcname) == 0) break;
    }
    fclose(fp);
    //printf("found hint = 0x%02X for %s\n", hint, funcname);
    assert(hint != -1);
    return hint;
}

void process_data()
{
    qsort(ilist, ilist_cnt, sizeof(struct importitem), ilist_cmp);
    
    printf("RVA of IMPORT is %08X, you may want to adjust this value\n", IMPORT_RVA);
    unsigned iatsize, importsize;
    int i;
    assert(ilist_cnt > 0);
    assert(ilist[0].ptraddr == IAT_START);
    for (i = 1; i < ilist_cnt; i++) {
        assert(strcmp(ilist[i].dllname, ilist[i - 1].dllname) != 0 || ilist[i].ptraddr - ilist[i - 1].ptraddr == 4);
    }
    for (i = 0; i < ilist_cnt; i++) {
        ilist[i].ptraddr = ilist[i].ptraddr - IAT_START + IAT_RVA;
    }
    iatsize = ilist[ilist_cnt - 1].ptraddr + 4 + 4 - ilist[0].ptraddr;
    printf("IAT SIZE %08X\n", iatsize);
    
    static unsigned char importdata[BUFSIZE];
    memset(importdata, 0, sizeof(importdata));
    PIMAGE_IMPORT_DESCRIPTOR piid = (void *) importdata;
    int dllcount = 0;
    for (i = 0; i < ilist_cnt; i++) {
        if (i == 0 || strcmp(ilist[i].dllname, ilist[i - 1].dllname) != 0) {
            dllcount++;
        }
        ilist[i].dllid = dllcount - 1;
    }

    unsigned char *iatblock = importdata + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dllcount + 1);
    unsigned char *strblock = iatblock + iatsize;
    unsigned strblocklen = 0;
    for (i = 0; i < ilist_cnt; i++) {
        if (i == 0 || strcmp(ilist[i].dllname, ilist[i - 1].dllname) != 0) {
            piid[ilist[i].dllid].Characteristics = IMPORT_RVA + (iatblock - importdata) + (ilist[i].ptraddr - IAT_RVA);
            piid[ilist[i].dllid].FirstThunk = ilist[i].ptraddr;
        }
        unsigned importoffset = IMPORT_RVA + (strblock + strblocklen - importdata);
        memcpy(iatblock + (ilist[i].ptraddr - IAT_RVA), &importoffset, sizeof(importoffset));
        unsigned short hint = find_hint(ilist[i].dllname, ilist[i].funcname);
        memcpy(strblock + strblocklen, &hint, 2);
        strcpy(strblock + strblocklen + 2, ilist[i].funcname);
        strblocklen += 2 + strlen(ilist[i].funcname) + 1;
        strblocklen = ROUND_UP(strblocklen, 2);
        if (i == ilist_cnt - 1 || strcmp(ilist[i].dllname, ilist[i + 1].dllname) != 0) {
            piid[ilist[i].dllid].Name = IMPORT_RVA + (strblock + strblocklen - importdata);
            unsigned dllnamelen = strlen(ilist[i].dllname);
            unsigned j;
            for (j = 0; j < dllnamelen; j++) {
                strblock[strblocklen + j] = toupper(ilist[i].dllname[j]);
            }
            strblock[strblocklen + dllnamelen] = '\0';
            strblocklen += dllnamelen + 1;
            strblocklen = ROUND_UP(strblocklen, 2);
        }
    }
    importsize = strblock + strblocklen - importdata;
    printf("IMPORT SIZE %08X\n", importsize);
    int ret;
    FILE *fp;
    fp = fopen("iat.bin", "wb");
    assert(fp);
    ret = fwrite(iatblock, 1, iatsize, fp);
    assert(ret == iatsize);
    fclose(fp);
    fp = fopen("import.bin", "wb");
    assert(fp);
    ret = fwrite(importdata, 1, importsize, fp);
    assert(ret == importsize);
    fclose(fp);
    /*for (i = 0; i < ilist_cnt; i++) {
        printf("%08X %s %s\n", ilist[i].ptraddr, ilist[i].funcname, ilist[i].dllname);
    }*/
}

void clean_up()
{
    int i;
    for (i = 0; i < ilist_cnt; i++) {
        free(ilist[i].funcname);
        free(ilist[i].dllname);
    }
}
int main()
{
    read_data();
    process_data();
    clean_up();
    return 0;
}
