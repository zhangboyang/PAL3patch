#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <openssl/sha.h>

/*
    the ZPK packer
    
    compile:
        gcc -O2 -Wall -o ZPKpacker ZPKpacker.c -lcrypto
    
    file format:
        [HEADER]: 'Z' 'P' 'K' '\0'
        [file_count]: sizeof(int)
        [fileinfo_list (index)]: sizeof(fileinfo_t) * file_count
        [filedata]
    
    the files will be saved in input order
    but the file index will be saved in hash order
*/
static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    putchar('\n');
    exit(1);
    va_end(ap);
}

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

#define MAXFILES 200000
#define MAXLINE 4096

char zpkheader[4] = "ZPK";

int file_cnt;
int filerank[MAXFILES];
char *filepath[MAXFILES];
struct fileinfo_t {
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned int offset; // relative to file start
    unsigned int len;
} fileinfo[MAXFILES];

int filerankcmp(const void *a, const void *b)
{
    int ranka = *(int *)a, rankb = *(int *)b;
    return memcmp(fileinfo[ranka].hash, fileinfo[rankb].hash, SHA_DIGEST_LENGTH);
}

int main(int argc, char *argv[])
{
    int i, j, rank;
    char buf[MAXLINE];
    
    // check parameters
    if (argc != 2) {
        printf("usage: ZPKpacker OUTPUTFILE\n");
        printf("       file list should passed by stdin\n");
        return 1;
    }
    
    // welcome message
    printf("ZBY's file package packer, built on %s, %s\n", __TIME__, __DATE__);
    
    // open output file for writing
    char *zpkfile = argv[1];
    printf("output file is '%s'\n", zpkfile);
    FILE *zpkfp = fopen(zpkfile, "wb");
    if (!zpkfp) fail("can't open '%s' for writing", zpkfile);
    snprintf(buf, sizeof(buf), "%s.list", zpkfile);
    printf("list file is '%s'\n", buf);
    FILE *lstfp = fopen(buf, "w");
    if (!lstfp) fail("can't open '%s' for writing", zpkfile);
    
    // load file list
    file_cnt = 0;
    while (scanf(" %" TOSTR(MAXLINE) "[^\n]", buf) == 1) {
        if (file_cnt >= MAXFILES) fail("too many files");
        filepath[file_cnt] = strdup(buf);
        file_cnt++;
    }
    
    
    // read file, calc file SHA1 hashs, calc file size
    printf("reading files ...\n");
    for (i = 0; i < file_cnt; i++) {
        SHA_CTX c;
        if (!SHA1_Init(&c)) fail("sha1 init failed");
        
        FILE *fp = fopen(filepath[i], "rb");
        if (!fp) fail("can't open file '%s'", filepath[i]);
        unsigned int filelen = 0;
        unsigned int len;
        while ((len = fread(buf, 1, sizeof(buf), fp))) {
            if (!SHA1_Update(&c, buf, len)) fail("sha1 update failed");
            filelen += len;
        }
        fclose(fp);
        SHA1_Final(fileinfo[i].hash, &c);
        fileinfo[i].len = filelen;
    }
    
    
    
    // calc offset
    unsigned header_size = sizeof(zpkheader) + sizeof(int) + file_cnt * sizeof(struct fileinfo_t);
    unsigned totsize = header_size;
    for (i = 0; i < file_cnt; i++) {
        fileinfo[i].offset = totsize;
        totsize += fileinfo[i].len;
    }
    

    // sort by hash
    for (i = 0; i < file_cnt; i++) {
        filerank[i] = i;
    }
    qsort(filerank, file_cnt, sizeof(int), filerankcmp);
    
    
    printf("writing files ...\n");
    
    
    // write header
    if (fwrite(zpkheader, 1, sizeof(zpkheader), zpkfp) != sizeof(zpkheader)) {
        fail("can't write zpkheader");
    }
    if (fwrite(&file_cnt, sizeof(int), 1, zpkfp) != 1) {
        fail("can't write file count");
    }
    for (rank = 0; rank < file_cnt; rank++) {
        i = filerank[rank];
        fwrite(&fileinfo[i], sizeof(struct fileinfo_t), 1, zpkfp);
    }
    assert(ftell(zpkfp) == header_size);
    
    // copy file contents
    for (i = 0; i < file_cnt; i++) {
        FILE *fp = fopen(filepath[i], "rb");
        if (!fp) fail("can't open file '%s'", filepath[i]);
        unsigned int len;
        while ((len = fread(buf, 1, sizeof(buf), fp))) {
            if (fwrite(buf, 1, len, zpkfp) != len) {
                fail("fwrite failed");
            }
        }
        fclose(fp);
    }
    assert(ftell(zpkfp) == totsize);
    
    
    // write index info to list
    fprintf(lstfp, "COUNT %d\n", file_cnt);
    fprintf(lstfp, "SIZE %08X\n", totsize);
    for (rank = 0; rank < file_cnt; rank++) {
        i = filerank[rank];
        fprintf(lstfp, "HASH ");
        for (j = 0; j < SHA_DIGEST_LENGTH; j++) {
            fprintf(lstfp, "%02x", fileinfo[i].hash[j]);
        }
        fprintf(lstfp, " OFFSET %08X LENGTH %08X PATH %s\n", fileinfo[i].offset, fileinfo[i].len, filepath[i]);
    }
    
    
    
    // free memory
    for (i = 0; i < file_cnt; i++) {
        free(filepath[i]);
    }
    
    fclose(lstfp);
    fclose(zpkfp);
    printf("%d files packed, %u bytes written.\n", file_cnt, totsize);
    return 0;
}
