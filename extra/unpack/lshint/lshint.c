// list 'hint' for LIB or DLL file
// see notes20160713.txt for details

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"

#define STR_STARTS_WITH(A, B) (strncmp((A), (B), strlen((B))) == 0)

#define TMPFILE "LSHINT.TMP"

static void check_no_space_chars(char *str)
{
    while (*str) { assert(!isspace(*str)); str++; }
}

int lsdll(char *dllname)
{
    int ret;
    char cmdline[MAXLINE];
    snprintf(cmdline, sizeof(cmdline), "DUMPBIN /EXPORTS /OUT:%s \"%s\"", TMPFILE, dllname);
    printf("RUN: %s\n", cmdline);
    ret = system(cmdline);
    assert(ret == 0);

    int funccnt = 0;
    char fnout[MAXLINE];
    char buf[MAXLINE];
    FILE *fp = fopen(TMPFILE, "r");
    assert(fp);
    snprintf(fnout, sizeof(fnout), "%s.hint.txt", dllname);
    FILE *fpout = fopen(fnout, "w");
    while (fgets(buf, sizeof(buf), fp) && !STR_STARTS_WITH(buf, "    ordinal hint RVA      name"));
    fgets(buf, sizeof(buf), fp); // read blank line
    while (fgets(buf, sizeof(buf), fp) && STR_STARTS_WITH(buf, "       ")) {
        unsigned hint;
        char funcname[MAXLINE];
        ret = sscanf(buf, "%*d%x%*x %[^\n]", &hint, funcname);
        assert(ret == 2);
        check_no_space_chars(funcname);
        fprintf(fpout, "%04X %s\n", hint, funcname);
        funccnt++;
    }
    fclose(fpout);
    fclose(fp);
    printf("TOTAL %d FUNCTIONS\n", funccnt);
    
    return 0;
}

int lslib(char *libname)
{
    int ret;
    char cmdline[MAXLINE];
    snprintf(cmdline, sizeof(cmdline), "DUMPBIN /HEADERS /OUT:%s \"%s\"", TMPFILE, libname);
    printf("RUN: %s\n", cmdline);
    ret = system(cmdline);
    assert(ret == 0);

    int funccnt = 0;
    char fnout[MAXLINE];
    char buf[MAXLINE];
    char *pcret;
    FILE *fp = fopen(TMPFILE, "r");
    assert(fp);
    snprintf(fnout, sizeof(fnout), "%s.hint.txt", libname);
    FILE *fpout = fopen(fnout, "w");
    while (fgets(buf, sizeof(buf), fp)) {
        if (STR_STARTS_WITH(buf, "  Hint         : ")) {
            unsigned hint;
            char funcname[MAXLINE];
            ret = sscanf(buf, "%*s%*s%u", &hint);
            assert(ret == 1);
            pcret = fgets(buf, sizeof(buf), fp);
            assert(pcret);
            ret = sscanf(buf, "%*s%*s %[^\n]", funcname);
            assert(ret == 1);
            check_no_space_chars(funcname);
            fprintf(fpout, "%04X %s\n", hint, funcname);
            funccnt++;
        }
    }
    fclose(fpout);
    fclose(fp);
    printf("TOTAL %d FUNCTIONS\n", funccnt);
    return 0;
}

int main(int argc, char *argv[])
{
    assert(argc >= 1);
    if (argc != 2) {
        printf("  usage: %s [DLLFILE|LIBFILE]\n", argv[0]);
        return 1;
    }
    remove(TMPFILE);
    int len = strlen(argv[1]);
    int ret;
    if (len > 4 && strnicmp(argv[1] + len - 4, ".dll", 4) == 0) {
        ret = lsdll(argv[1]);
    } else if (len > 4 && strnicmp(argv[1] + len - 4, ".lib", 4) == 0) {
        ret = lslib(argv[1]);
    } else {
        printf("  file format not recognized\n");
        return 1;
    }
    remove(TMPFILE);
    return ret;
}
