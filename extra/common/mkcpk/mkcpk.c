// this program can make uncompressed CPK files
//  usage:
//    mkcpk.exe [CPKFILE]
//      then put a file list to stdin, like:
//               dir1/dir2/file1
//               dir3/file2
//               file3
//      the order of files in list will be the order in CPK


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef unsigned long ULONG;

#define fail(fmt, ...) (fprintf(stderr, "  FATAL: " fmt "\n", ##__VA_ARGS__), exit(1))
#define warning(fmt, ...) (fprintf(stderr, "  WARNING: " fmt "\n", ##__VA_ARGS__))

static int cmp_stricmp(const void *a, const void *b)
{
    return _stricmp(*(const char **)a, *(const char **)b);
}

// gbCrc32 implementation
static unsigned CrcTbl[0x100];
static void gbCrc32Init()
{
	unsigned v0 = 0;
	unsigned *v1 = CrcTbl;
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
	} while (v1 < CrcTbl + 0x100);
}

static unsigned gbCrc32Compute(const char *str)
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
		x = CrcTbl[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}



// these structures are read from PAL3A.PDB
struct CPKHeader {
    ULONG dwLable;
    ULONG dwVersion;
    ULONG dwTableStart;
    ULONG dwDataStart;
    ULONG dwMaxFileNum;
    ULONG dwFileNum;
    ULONG dwIsFormatted;
    ULONG dwSizeOfHeader;
    ULONG dwValidTableNum;
    ULONG dwMaxTableNum;
    ULONG dwFragmentNum;
    ULONG dwPackageSize;
    ULONG dwReserved[0x14];
};
struct CPKTable {
    ULONG dwCRC;
    ULONG dwFlag;
    ULONG dwFatherCRC;
    ULONG dwStartPos;
    ULONG dwPackedSize;
    ULONG dwOriginSize;
    ULONG dwExtraInfoSize;
};

#define MAXLINE 4096
#define DIR_SEPARATOR "/\\"
#define CPK_MAXTABLENUM 0x8000

static struct CPKHeader cpkhdr;
static struct CPKTable cpktbl[CPK_MAXTABLENUM];

static char filelist[CPK_MAXTABLENUM][MAXLINE];
static char *filelist_sorted[CPK_MAXTABLENUM];
static int nr_files;

static size_t fwrite_safe(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t r = fwrite(ptr, size, nmemb, stream);
    assert(r == nmemb);
    return r;
}

static int dircmp(const char *a, const char *b)
{
    while (*a && *a != '\\' && _strnicmp(a, b, 1) == 0) a++, b++;
    char cha = *a, chb = *b;
    if (cha == '\\') cha = '\0';
    if (chb == '\\') chb = '\0';
    return _strnicmp(&cha, &chb, 1);
}

static int tblcmp(const void *a, const void *b)
{
    const struct CPKTable *ta = a, *tb = b;
    if (ta->dwCRC < tb->dwCRC) return -1;
    if (ta->dwCRC > tb->dwCRC) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    FILE *fp;
    char buf[MAXLINE];
    
    // init gbCrc32
    gbCrc32Init();
    
    // parse cpk file name
    const char *cpkfn = "newcpk.cpk";
    if (argc >= 2) cpkfn = argv[1];
    else warning("cpk file name default to '%s'", cpkfn);
    
    // read file list
    nr_files = 0;
    while (fgets(buf, sizeof(buf), stdin)) {
        char *begin = buf;
        char *end = buf + strlen(buf);
        while (*begin && (isspace(*begin) || strchr(DIR_SEPARATOR, *begin))) begin++;
        while (end > begin && isspace(end[-1])) *--end = 0;
        if (end > begin && *begin != ';' && *begin != '#') {
            strcpy(filelist[nr_files++], begin);
        }
    }
    if (nr_files == 0) fail("file list is empty.");
    
    // sort file list
    for (i = 0; i < nr_files; i++) {
        filelist_sorted[i] = filelist[i];
    }
    qsort(filelist_sorted, nr_files, sizeof(char *), cmp_stricmp);
    
    // open cpk file for write
    fp = fopen(cpkfn, "wb");
    
    // write dummy header and crc table
    //   will be overwrited later
    memset(&cpkhdr, 0, sizeof(cpkhdr));
    memset(&cpktbl, 0, sizeof(cpktbl));
    fwrite_safe(&cpkhdr, sizeof(cpkhdr), 1, fp);
    fwrite_safe(&cpktbl, sizeof(cpktbl), 1, fp);
    
    // make dummy data for extrainfo
    char extrainfo[0x3A];
    memset(extrainfo, 0, 2);
    memset(extrainfo + 2, 0xCC, 0x38);

    // make dir entry
    char dirstr[MAXLINE];
    strcpy(dirstr, "");
    char *dirstack[MAXLINE];
    dirstack[0] = dirstr;
    unsigned dircrc[MAXLINE];
    int nr_tbl = 0;
    for (i = 0; i < nr_files; i++) {
        char *token;
        int lvl;
        int tot_lvl;
        int last_lvl = -1;
        unsigned tblflag;

        // split path and compare to dir stack
        strcpy(buf, filelist_sorted[i]);
        for (token = strtok(buf, DIR_SEPARATOR), lvl = 0; token; lvl++) {
            assert(*token);
            if (last_lvl < 0) {
                if (dircmp(dirstack[lvl], token) != 0) {
                    last_lvl = lvl;
                    strcpy(dirstack[lvl], token);
                    token = strtok(NULL, DIR_SEPARATOR);
                } else {
                    token = strtok(NULL, DIR_SEPARATOR);
                    continue;
                }
            } else {
                dirstack[lvl] = dirstack[lvl - 1];
                strcat(dirstack[lvl], "\\");
                dirstack[lvl] += strlen(dirstack[lvl]);
                strcpy(dirstack[lvl], token);
                token = strtok(NULL, DIR_SEPARATOR);
            }
            unsigned offset = 0, extsize = 0;
            if (token) {
                // create dir
                printf("  create dir entry for '%s'\n", dirstr);
                tblflag = 0x00000003;
                
                // parse base name
                char *basename = strrchr(buf, '\\');
                if (basename) basename++;
                else basename = buf;
                
                // write extrainfo
                offset = ftell(fp);
                fwrite_safe(basename, 1, strlen(basename), fp);
                fwrite_safe(extrainfo, 1, sizeof(extrainfo), fp);
                extsize = strlen(basename) + sizeof(extrainfo);
                
            } else {
                // create file
                printf("  create file entry for '%s'\n", dirstr);
                tblflag = 0x00010005;
            }
            _strlwr(dirstr);
            dircrc[lvl] = gbCrc32Compute(dirstr);
            if (nr_tbl >= CPK_MAXTABLENUM) {
                fail("too many table entries.");
            }
            cpktbl[nr_tbl++] = (struct CPKTable) {
                .dwCRC = dircrc[lvl],
                .dwFlag = tblflag,
                .dwFatherCRC = lvl > 0 ? dircrc[lvl - 1] : 0,
                .dwStartPos = offset,
                .dwPackedSize = 0,
                .dwOriginSize = 0,
                .dwExtraInfoSize = extsize,
            };
        }
        tot_lvl = lvl - 1;
        assert(last_lvl >= 0);
    }
    
    // sort table entries
    qsort(cpktbl, nr_tbl, sizeof(struct CPKTable), tblcmp);
    for (i = 1; i < nr_tbl; i++) {
        if (cpktbl[i - 1].dwCRC == cpktbl[i].dwCRC) {
            fail("duplicate filename CRC.");
        }
    }


    // copy file data
    for (i = 0; i < nr_files; i++) {
        char *fpath = filelist[i];
        // normalize path
        strcpy(buf, fpath);
        char *dst = buf, *src = buf;
        while (*src) {
            if (strchr(DIR_SEPARATOR, *src)) {
                while (*src && strchr(DIR_SEPARATOR, *src)) src++;
                if (*src) {
                    *dst++ = '\\';
                } else {
                    *dst = '\0';
                    break;
                }
            } else {
                *dst++ = *src++;
            }
        }
        
        // parse base name
        char fn[MAXLINE];
        char *basename = strrchr(buf, '\\');
        if (basename) basename++;
        else basename = buf;
        strcpy(fn, basename);
        
        // convert to lower case and compute hash
        _strlwr(buf);
        struct CPKTable tblkey;
        tblkey = (struct CPKTable) {
            .dwCRC = gbCrc32Compute(buf),
        };
        
        // binary search in table
        struct CPKTable *p;
        p = bsearch(&tblkey, cpktbl, nr_tbl, sizeof(struct CPKTable), tblcmp);
        
        // copy data
        p->dwStartPos = ftell(fp);
        FILE *datafp = fopen(fpath, "rb");
        if (!datafp) fail("can't open '%s'.", fpath);
        int cnt;
        int r;
        unsigned filesz = 0;
        char databuf[MAXLINE];
        while ((cnt = fread(databuf, 1, sizeof(databuf), datafp)) > 0) {
            r = fwrite_safe(databuf, 1, cnt, fp);
            filesz += r;
        }
        fclose(datafp);
        
        p->dwPackedSize = p->dwOriginSize = filesz;
        
        // write extrainfo
        fwrite_safe(fn, 1, strlen(fn), fp);
        fwrite_safe(extrainfo, 1, sizeof(extrainfo), fp);
        p->dwExtraInfoSize = strlen(fn) + sizeof(extrainfo);
        
        printf("  copy data for '%s'\n", buf);
    }
    
    
    // make cpk header
    cpkhdr = (struct CPKHeader) {
        .dwLable = 0x1A545352,
        .dwVersion = 1,
        .dwTableStart = sizeof(struct CPKHeader),
        .dwDataStart = sizeof(struct CPKHeader) + sizeof(struct CPKTable) * CPK_MAXTABLENUM,
        .dwMaxFileNum = CPK_MAXTABLENUM,
        .dwFileNum = nr_tbl,
        .dwIsFormatted = 0,
        .dwSizeOfHeader = sizeof(struct CPKHeader),
        .dwValidTableNum = nr_tbl,
        .dwMaxTableNum = CPK_MAXTABLENUM,
        .dwFragmentNum = 0,
        .dwPackageSize = ftell(fp),
    };
    memset(cpkhdr.dwReserved, 0, sizeof(cpkhdr.dwReserved));
    
    // write cpk header
    rewind(fp);
    fwrite_safe(&cpkhdr, sizeof(cpkhdr), 1, fp);
    fwrite_safe(&cpktbl, sizeof(cpktbl), 1, fp);
    
    fclose(fp);
    printf("    %d file(s) packed.\n", nr_files);

    return 0;
}
