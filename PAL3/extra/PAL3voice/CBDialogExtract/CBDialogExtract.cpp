#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <set>


static unsigned crctbl[0x100];
static void init_crc32()
{
    unsigned v0 = 0;
    unsigned *v1 = crctbl;
    unsigned v2;
    unsigned v3;
    do {
        v2 = 8;
        v3 = v0 << 24;
        do {
            if (v3 & 0x80000000) {
                v3 = 2 * v3 ^ 0x04C11DB7;
            } else {
                v3 *= 2;
            }
            --v2;
        } while (v2);
        *v1 = v3;
        ++v1;
        ++v0;
    } while (v1 < crctbl + 0x100);
}
unsigned gbCrc32Compute(const char *str)
{
    unsigned char *s = (unsigned char *) str;
    unsigned x = 0;
    int i;
    for (i = 0; i < 4; i++) {
        x = (x << 8);
        if (*s) x |= *s++;
    }
    x = ~x;
    while (*s) {
        x = crctbl[x >> 24] ^ (*s++ | (x << 8));
    }
    return ~x;
}





// main program

#define MAXLINE 4096
int main()
{
    init_crc32();
    
    std::set<std::string> txtset;
    FILE *fp = fopen("combat.ini", "r");
    FILE *fpout = fopen("CBDialogData.txt", "w");
    char buf[MAXLINE];
    char role[MAXLINE];
    char out[MAXLINE * 2];
    role[0] = '\0';
    while (fgets(buf, sizeof(buf), fp)) {
        char *nl = strchr(buf, '\n');
        if (nl) *nl = '\0';
        if (strstr(buf, "-CombatDialog]")) {
            sscanf(buf, "[%[^-]", role);
        }
        if (role[0]) {
            char *val = strchr(buf, '=');
            if (val) {
                char *key = buf;
                *val++ = '\0';
                //fprintf(fpout, "ROLE=%s KEY=%s VAL=%s\n", role, key, val);
                sprintf(out, "CBD|%s", val);
                fprintf(fpout, "CBDialog\\%s_%s.wav|1.000|:|%08X|%s\n", role, key, gbCrc32Compute(out), out);
                if (!txtset.insert(std::string(val)).second) {
                    printf("WARNING: duplicate string '%s'.\n", val);
                }
            }
        }
    }
    fclose(fp);
    fclose(fpout);
    system("pause");
    return 0;
}
