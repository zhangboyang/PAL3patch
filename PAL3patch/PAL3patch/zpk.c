#include "common.h"

#define SHA1_BYTE 20
#define SHA1_STR_SIZE (SHA1_BYTE * 2 + 1)

#define ZPK_HEADERSIZE 4
#define ZPK_MAXFILES 200000


static FILE *zpkfp = NULL;
static unsigned zpkcount;

struct fileinfo_t {
    unsigned char hash[SHA1_BYTE];
    unsigned int offset;
    unsigned int len;
};

static struct fileinfo_t zpkfi[ZPK_MAXFILES];

static int zpkcmp(const void *a, const void *b)
{
    const struct fileinfo_t *pa = a, *pb = b;
    return memcmp(pa->hash, pb->hash, SHA1_BYTE);
}

void load_zpk(const char *zpkfn)
{
    // open file
    zpkfp = fopen64(zpkfn, "rb");
    if (!zpkfp) {
        fail("can't open zpk file %s.", zpkfn);
    }
    
    // load index
    if (fseeko64(zpkfp, ZPK_HEADERSIZE, SEEK_SET) != 0) {
        fail("can't seek after zpk file header.");
    }
    
    // read file count
    if (fread(&zpkcount, sizeof(int), 1, zpkfp) != 1) {
        fail("can't read file count.");
    }
    if (zpkcount > ZPK_MAXFILES) {
        fail("too many files in zpk.");
    }
    
    // read file info
    if (fread(zpkfi, sizeof(struct fileinfo_t), zpkcount, zpkfp) != zpkcount) {
        fail("can't load zpk index.");
    }
    
    int i;
    for (i = 1; i < zpkcount; i++) {
        if (zpkcmp(&zpkfi[i], &zpkfi[i - 1]) < 0) {
            fail("unsorted index in zpk.");
        }
    }
}

// don't forget to free memory when calling zpk_getfile()
int zpk_getfile(const void *sha1buf, void **dataptr, unsigned *datalen)
{
    struct fileinfo_t key;
    memcpy(key.hash, sha1buf, SHA1_BYTE);
    struct fileinfo_t *ptr = bsearch(&key, zpkfi, zpkcount, sizeof(struct fileinfo_t), zpkcmp);
    if (!ptr) {
        plog("file not found for sha1 '%s'.", sha1_tostr(sha1buf));
        return 0;
    }
    if (fseeko64(zpkfp, ptr->offset, SEEK_SET) != 0) {
        plog("failed to seek zpk.");
        return 0;
    }
    *datalen = ptr->len;
    *dataptr = malloc(ptr->len);
    if (!*dataptr) {
        plog("failed to allocate buffer.");
        return 0;
    }
    if (fread(*dataptr, 1, *datalen, zpkfp) != *datalen) {
        plog("failed to read zpk.");
        free(*dataptr);
        *dataptr = NULL;
        return 0;
    }
    
    unsigned char datahash[SHA1_BYTE];
    sha1_hash_buffer(*dataptr, *datalen, datahash);
    if (memcmp(sha1buf, datahash, SHA1_BYTE) != 0) {
        plog("corrupt data in zpk entry '%s'.", sha1_tostr(sha1buf));
        free(*dataptr);
        *dataptr = NULL;
        return 0;
    }
    return 1;
}

void unload_zpk()
{
    zpkcount = 0;
    if (zpkfp) {
        fclose(zpkfp);
        zpkfp = NULL;
    }
}
