#include "common.h"

#define TEXPACK_MAXFILES 200000
#define TEXPACK_MAXCPKS 100

// uncomment to find out fake full transparent images
//#define DETECT_FAKE_FULL_TRANSPARENT_IMAGES



struct tpinfo_t {
    char *cpkname;
    char *fpath;
    unsigned char cpkhash[SHA1_BYTE]; // original hash
    unsigned char zpkhash[SHA1_BYTE]; // new hash
};

static int tpinfo_cmp(const void *a, const void *b)
{
    const struct tpinfo_t *pa = a, *pb = b;
    int ret;
    ret = stricmp(pa->cpkname, pb->cpkname);
    if (!ret) ret = stricmp(pa->fpath, pb->fpath);
    return ret;
}

#ifdef DETECT_FAKE_FULL_TRANSPARENT_IMAGES
static int tpinfo_cmp_by_zpkhash(const void *a, const void *b)
{
    const struct tpinfo_t *pa = a, *pb = b;
    return memcmp(pa->zpkhash, pb->zpkhash, SHA1_BYTE);
}
#endif

static struct tpinfo_t tpinfo[TEXPACK_MAXFILES];
static int tpcount = 0;

static char *cpklist[TEXPACK_MAXCPKS];
static int cpkcount = 0;

static void load_texture_pack(const char *tpname)
{
    char buf[MAXLINE];

    FILE *fp = fopen(tpname, "r");
    if (!fp) fail("can't open texture pack '%s'.", tpname);
    
    if (fscanf(fp, "ZPK \"%" TOSTR(MAXLINE) "[^\"]\"", buf) != 1) {
        fail("can't read zpk name from texture pack.");
    }
    load_zpk(buf);
    
    tpcount = 0;
    cpkcount = 0;
    char *tpcpk = NULL;
    while (fscanf(fp, " " MAXLINEFMT, buf) == 1) {
        if (tpcpk && strcmp(buf, "FILE") == 0) {
            struct tpinfo_t *ptr = &tpinfo[tpcount];
            ptr->cpkname = tpcpk;
            if (fscanf(fp, " \"%" TOSTR(MAXLINE) "[^\"]\"", buf) != 1) {
                fail("can't read original file name");
            }
            ptr->fpath = strdup(buf);
            if (fscanf(fp, " SHA1 " MAXLINEFMT, buf) != 1) {
                fail("can't read first hash.");
            }
            sha1_fromstr(ptr->cpkhash, buf);
            if (fscanf(fp, " MAPTO SHA1 " MAXLINEFMT, buf) != 1) {
                fail("can't read second hash.");
            }
            sha1_fromstr(ptr->zpkhash, buf);
            tpcount++;
        } else if (!tpcpk && strcmp(buf, "BEGIN") == 0) {
            if (fscanf(fp, " CPK \"%" TOSTR(MAXLINE) "[^\"]\"", buf) != 1) {
                fail("can't read CPK name.");
            }
            tpcpk = cpklist[cpkcount++] = strdup(buf);
        } else if (tpcpk && strcmp(buf, "END") == 0) {
            tpcpk = NULL;
            if (fscanf(fp, " " MAXLINEFMT, buf) != 1 || strcmp(buf, "CPK") != 0) {
                fail("unexpected token after END.");
            }
        } else {
            fail("unexpected token %s.", buf);
        }
    }
    
    qsort(tpinfo, tpcount, sizeof(struct tpinfo_t), tpinfo_cmp);
    
    fclose(fp);
}

// don't forget to free memory when calling get_alternative_texture()
static int get_alternative_texture(const char *cpkname, const char *fpath, void *olddata, unsigned oldlen, void **newdata, unsigned *newlen)
{
    struct tpinfo_t key;
    key.cpkname = (char *) cpkname;
    key.fpath = (char *) fpath;
    struct tpinfo_t *ptr = bsearch(&key, tpinfo, tpcount, sizeof(struct tpinfo_t), tpinfo_cmp);
    if (!ptr) {
        plog("texpack entry '%s\\%s' not found.", cpkname, fpath);
        return 0;
    }
    if (olddata) {
        unsigned char hashbuf[SHA1_BYTE];
        sha1_hash_buffer(olddata, oldlen, hashbuf);
        if (memcmp(ptr->cpkhash, hashbuf, SHA1_BYTE) != 0) {
            plog("hash mismatch for cpk texture '%s\\%s'.", cpkname, fpath);
            return 0;
        }
    }
    return zpk_getfile(ptr->zpkhash, newdata, newlen);
}

static void unload_texture_pack()
{
    // free memory
    int i;
    for (i = 0; i < cpkcount; i++) {
        free(cpklist[i]);
    }
    for (i = 0; i < tpcount; i++) {
        free(tpinfo[i].fpath);
    }
    cpkcount = tpcount = 0;
    
    unload_zpk();
}

void *load_png(void *fdata, unsigned flen, int *width, int *height, int *bitcount, malloc_funcptr_t pmalloc, free_funcptr_t pfree)
{
    void *ret = NULL;
    unsigned pbits_size;
    void *pbits = NULL;
    struct D3DXTex_CImage img;
    D3DXTex_CImage_ctor(&img);
    
    if (D3DXTex_CImage_Load(&img, fdata, flen, NULL, 1) < 0) {
        goto done;
    }
    
    *width = img.Width;
    *height = img.Height;
    
    switch (img.Format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_P8:
        case D3DFMT_A8L8:
            *bitcount = 32;
            break;
        case D3DFMT_X8R8G8B8:
        case D3DFMT_L8:
            *bitcount = 24;
            break;
        default:
            plog("unknown D3DFORMAT %d.", (int) img.Format);
            goto done;
    }
    
    
    pbits_size = (*width) * (*height) * (*bitcount / 8);
    pbits = pmalloc(pbits_size);
    
    
    int i, p, b;
    int pitch = (*width) * (*bitcount / 8);
    for (i = 0; i < *height; i++) {
        unsigned char *src = img.pBits + img.Pitch * i;
        unsigned char *dest = pbits + pitch * ((*height) - i - 1);
        switch (img.Format) {
            // 32 bit
            case D3DFMT_A8R8G8B8:
                memcpy(dest, src, pitch);
                break;
            case D3DFMT_P8:
                for (p = 0; p < *width; p++) {
                    unsigned char *c = (void *) &img.pPalette[src[p]];
                    dest[p * 4 + 0] = c[2]; // need byte swap
                    dest[p * 4 + 1] = c[1];
                    dest[p * 4 + 2] = c[0];
                    dest[p * 4 + 3] = c[3];
                }
                break;
            case D3DFMT_A8L8:
                for (p = 0; p < *width; p++) { // need byte swap
                    for (b = 0; b < 3; b++) {
                        dest[p * 4 + b] = src[p * 2];
                    }
                    dest[p * 4 + 3] = src[p * 2 + 1];
                }
                break;
            
            // 24 bit
            case D3DFMT_X8R8G8B8:
                for (p = 0; p < *width; p++) {
                    for (b = 0; b < 3; b++) {
                        dest[p * 3 + b] = src[p * 4 + b];
                    }
                }
                break;
            case D3DFMT_L8:
                for (p = 0; p < *width; p++) {
                    for (b = 0; b < 3; b++) {
                        dest[p * 3 + b] = src[p];
                    }
                }
                break;
        }
    }

    // fill random color for testing purpose
    /*for (p = 0; p < (*height) * (*width); p++) {
        unsigned char *c = pbits + p * (*bitcount / 8);
        c[0] = rand(); c[1] = rand(); c[2] = rand();
    }*/
    
    ret = pbits;
done:
    
    if (!ret) gbfree(pbits); // free(NULL) is allowed
    D3DXTex_CImage_dtor(&img);
    return ret;
}

char *adjust_cpkname(char *cpkname, char *cpknamebuf, int cpknamebuf_len)
{
    if (cpkname && (str_iendwith(cpkname, "\\") || str_iendwith(cpkname, "/"))) {
        // cpkname is illegal, possiblly nocpk is enabled
        // try convert to legal cpkname
        snprintf(cpknamebuf, cpknamebuf_len, "%.*s.cpk", strlen(cpkname) - 1, cpkname);
        cpkname = cpknamebuf;
    }
    
    // cut cpkname
    if (cpkname) {
        char *separator = strrchr(cpkname, '\\');
        if (separator) cpkname = separator + 1;
    }
    
    return cpkname;
}

static void __fastcall gbTexture_D3D_CreateFromFileMemory_wrapper(struct gbTexture_D3D *this, int dummy, void *pSrcData, int SrcDataSize)
{
    char *fpath = TOPTR(gboffset + 0x10140C68);
    char *cpkname = g_pVFileSys->rtDirectory;
    
    char cpknamebuf[MAXLINE];
    cpkname = adjust_cpkname(cpkname, cpknamebuf, sizeof(cpknamebuf));

    void *fdataptr;
    unsigned fdatalen;
    void *mem_tobefree = NULL;
    
    if (cpkname && get_alternative_texture(cpkname, fpath, pSrcData, SrcDataSize, &fdataptr, &fdatalen)) {
        mem_tobefree = fdataptr;
    } else {
        // failed to load
        
        if (cpkname) {
            plog("can't find corresponding texture for '%s\\%s'.", cpkname, fpath);
        }
        
        // fallback to default
        fdataptr = pSrcData;
        fdatalen = SrcDataSize;
    }

    gbTexture_D3D_CreateFromFileMemory(this, fdataptr, fdatalen);
    free(mem_tobefree);
}

static MAKE_ASMPATCH(gbimage2d_loadimage_hook)
{
    R_ESP += 0x10; // oldcode

    int is_tga = !R_EAX;
    struct gbImage2D *this = TOPTR(R_EBP);
    char *fpath = TOPTR(R_EBX);
    char *cpkname = g_pVFileSys->rtDirectory;
    
    char cpknamebuf[MAXLINE];
    cpkname = adjust_cpkname(cpkname, cpknamebuf, sizeof(cpknamebuf));
    
    // file data buffer and length
    void *fdataptr;
    unsigned fdatalen;
    
    int flag = 0; // success flag
    
    if (cpkname) { // check if we are using CPK, for example, loading save/PAL3_abcd.JPG will not use CPK
        if (get_alternative_texture(cpkname, fpath, NULL, 0, &fdataptr, &fdatalen)) { // find alternative texture in ZPK
            int width, height, bitcount;
            void *pbits = load_png(fdataptr, fdatalen, &width, &height, &bitcount, gbmalloc, gbfree); // uncompress PNG
            if (pbits) {
                this->Width = width;
                this->Height = height;
                this->BitCount = bitcount;
                this->pBits = pbits;
                flag = 1; // set success flag
                M_DWORD(R_ESP + 0x10) = 2; // set type to TGA
                RETNADDR = gboffset + 0x1001E6C9; // finish image loading, jump to processing
            } // else success flag is still 0
            free(fdataptr); // free file contents        
        }
    }
    
    if (!flag) {
        // failed to find corresponding texture
        // fall back to default
        if (cpkname) {
            plog("can't find corresponding texture for '%s\\%s'.", cpkname, fpath);
        }
        
        if (!is_tga) { // oldcode
            RETNADDR = gboffset + 0x1001E544; // jump to next extension checking
        } // else fallthrough the TGA processing procdure
    }
}

MAKE_PATCHSET(replacetexture)
{
    load_texture_pack(get_string_from_configfile("texpack"));

#ifdef DETECT_FAKE_FULL_TRANSPARENT_IMAGES
    do {
        // re-sort tpinfo[] to speed up (random harddisk seeking is SLOW!)
        qsort(tpinfo, tpcount, sizeof(struct tpinfo_t), tpinfo_cmp_by_zpkhash);
        
        int ffti_count = 0, ffti_full_count = 0;
        FILE *fftifp = fopen("ffti.log", "w");
        int i;
        for (i = 0; i < tpcount; i++) {
            if (i % 1000 == 0) {
                char buf[MAXLINE];
                sprintf(buf, "FFTI: progress %d/%d\n", i, tpcount);
                OutputDebugString(buf);
            }
            void *fdataptr;
            unsigned fdatalen;
            int width, height, bitcount;
            
            zpk_getfile(tpinfo[i].zpkhash, &fdataptr, &fdatalen);
            void *pbits = load_png(fdataptr, fdatalen, &width, &height, &bitcount, malloc, free);
            
            if (bitcount == 32) {
                int p;
                int bad_flag = 0;
                int alpha_flag = 0;
                for (p = 0; p < height * width; p++) {
                    unsigned char *c = pbits + p * 4;
                    if (!c[3]) {
                        alpha_flag++;
                        if (c[0] || c[1] || c[2]) bad_flag++;
                    }
                    
                }
                if (bad_flag) {
                    int fullflag = (alpha_flag == height * width);
                    
                    fprintf(fftifp, "TYPE %s RATIO %.6f BAD %d TOTAL %d SHA1 %s CPK \"%s\" PATH \"%s\"\n", (fullflag ? "FULL" : "PART"), ((double) bad_flag / (height * width)), bad_flag, height * width, sha1_tostr(tpinfo[i].cpkhash), tpinfo[i].cpkname, tpinfo[i].fpath);
                    ffti_count++;
                    ffti_full_count += fullflag;
                }
            }
            
            free(pbits);
            free(fdataptr);
        }
        fclose(fftifp);
        warning("found %d fake transparent images, %d are full transparent.", ffti_count, ffti_full_count);
        
        qsort(tpinfo, tpcount, sizeof(struct tpinfo_t), tpinfo_cmp);
    } while (0);
#endif
    
    // hook gbImage2D::LoadImage
    INIT_ASMPATCH(gbimage2d_loadimage_hook, gboffset + 0x1001E517, 7, "\x83\xC4\x10\x85\xC0\x75\x26");
    
    // hook gbTexture::LoadTexture
    SIMPLE_PATCH(gboffset + 0x10020658, "\x8B\x13\x50\x8B\x86\x30\x01\x00\x00\x8B", "\x89\xD9\x50\x8B\x86\x30\x01\x00\x00\x50", 0xA);
    SIMPLE_PATCH(gboffset + 0x100206A4, "\x8B\x13", "\x89\xD9", 0x2);
    INIT_WRAPPER_CALL(gbTexture_D3D_CreateFromFileMemory_wrapper, {
            gboffset + 0x10020662,
            gboffset + 0x100206A8,
    });
    
    // add cleanup hook
    add_atexit_hook(unload_texture_pack);
}
