#include "common.h"

static void dds_loader(struct texture_hook_info *thinfo)
{
    void *fdataptr = NULL;
    void *bits = NULL;
    
    // check if image already loaded
    if (thinfo->bits) goto done;
    
    // check if we should load image
    if (test_texture_hook_noautoload(thinfo)) goto done;
    
    // replace extension to .dds
    char dds_fpath[MAXLINE];
    strcpy(dds_fpath, thinfo->loadpath);
    if (!strrchr(dds_fpath, '.')) goto done;
    strcpy(strrchr(dds_fpath, '.'), ".dds");
    
    // try read dds file
    unsigned fdatalen;
    fdataptr = vfs_readfile(dds_fpath, &fdatalen, thinfo->mem_allocator);
    if (!fdataptr) goto done;
    
    // try load dds file
    int width, height, bitcount;
    bits = load_image_bits(fdataptr, fdatalen, &width, &height, &bitcount, thinfo->mem_allocator);
    if (bits) {
        thinfo->bits = bits;
        thinfo->width = width;
        thinfo->height = height;
        thinfo->bitcount = bitcount;
        thinfo->div_alpha = 0;
        bits = NULL;
    }
    
done:
    thinfo->mem_allocator->free(fdataptr);
    thinfo->mem_allocator->free(bits);
}



// magic format: ":some_magic:extra_info"
// compare if thinfo have magic
int test_texture_hook_magic(struct texture_hook_info *thinfo)
{
    return thinfo->texpath[0] == ':';
}
// compare thinfo with given magic
// if given magic found, return pointer to extra info
// else return NULL
char *extract_texture_hook_magic(struct texture_hook_info *thinfo, const char *magic)
{
    if (thinfo->texpath[0] == ':' && strncmp(thinfo->texpath + 1, magic, strlen(magic)) == 0 && thinfo->texpath[1 + strlen(magic)] == ':') {
        return thinfo->texpath + 1 + strlen(magic) + 1;
    } else {
        return NULL;
    }
}
// make a magic string, return string in static buffer
const char *make_texture_hook_magic(const char *magic)
{
    static char buf[MAXLINE];
    strcpy(buf, ":");
    strcat(buf, magic);
    strcat(buf, ":");
    return buf;
}


// gbImage2D::LoadImageFile wants '.' in texpath
int test_texture_hook_noautoload(struct texture_hook_info *thinfo)
{
    return strcmp(thinfo->loadpath, ".") == 0;
}
void mark_texture_hook_noautoload(struct texture_hook_info *thinfo)
{
    strcpy(thinfo->loadpath, ".");
}



static int nr_texhooks = 0;
static void (*texhooks[MAX_TEXTURE_HOOKS])(struct texture_hook_info *);

void add_texture_hook(void (*funcptr)(struct texture_hook_info *))
{
    if (nr_texhooks >= MAX_TEXTURE_HOOKS) fail("too many texture hooks.");
    texhooks[nr_texhooks++] = funcptr;
}
static void run_texture_hooks(struct texture_hook_info *thinfo)
{
    int i;
    for (i = 0; i < nr_texhooks; i++) {
        texhooks[i](thinfo);
    }
}




static struct texture_hook_info g_thinfo;

static MAKE_ASMPATCH(texhook_part1)
{
    struct gbTexture *this = TOPTR(R_EBX);
    struct gbVFile *fp = TOPTR(R_EAX);
    const char *texpath;
    if (fp) {
        // if DDS is successfully opened, the use DDS as texture file name
        texpath = TOPTR(gboffset + 0x101155C8);
    } else {
        // otherwise use original filename as texture file name
        texpath = this->pName;
    }
    
    // fill thinfo
    struct texture_hook_info *thinfo = &g_thinfo;
    strcpy(thinfo->cpkname, vfs_cpkname());
    strcpy(thinfo->texpath, texpath);
    thinfo->mem_allocator = &gb_mem_allocator;
    thinfo->type = TH_PRE_IMAGELOAD;
    strcpy(thinfo->loadpath, texpath);
    thinfo->interested = 0;
    thinfo->div_alpha = 1;
    thinfo->bits = NULL;
    thinfo->width = 0;
    thinfo->height = 0;
    thinfo->bitcount = 0;
    thinfo->fakewidth = 0;
    thinfo->fakeheight = 0;
    
    // run hooks
    run_texture_hooks(thinfo);
    
    if (thinfo->interested) {
        if (fp) {
            // if some hooks interested in current texture
            // and DDS is successfully opened, close it
            gbVFileSystem_CloseFile(g_pVFileSys, fp);
            fp = NULL;
        }
        
        // call DDS loader to load image
        dds_loader(thinfo);
    }
    
    // oldcode
    R_ESI = R_EAX = TOUINT(fp);
    R_EDI = 0;
    if (!fp) {
        RETNADDR = gboffset + 0x10020201; // jump to gbImage2D loader
    } // else fallthrough to D3DX texture loading
}

// asmpatch should only be effective when gbImage2D::LoadImageFile
// is call from gbTexture::LoadTexture
#define EXPECTED_GBIMAGE2D_LOADIMAGEFILE_RETNADDR (gboffset + 0x10020244)

static MAKE_ASMPATCH(texhook_part2)
{
    const char *texpath = TOPTR(M_DWORD(R_EBP + 0x8));
    
    // check caller's address first
    if (M_DWORD(R_EBP + 0x4) != EXPECTED_GBIMAGE2D_LOADIMAGEFILE_RETNADDR) {
        goto done;
    }
    
    // load new name
    struct texture_hook_info *thinfo = &g_thinfo;
    texpath = thinfo->loadpath;
    
done:
    // oldcode
    R_EDI = M_DWORD(R_EBP + 0x8) = TOUINT(texpath);
    R_ECX = 0xFFFFFFFF;
}

static MAKE_ASMPATCH(texhook_part3)
{
    struct gbImage2D *this = TOPTR(R_EBX);
    int flag = 0; // imaged loaded flag
    
    // check caller's address first
    if (M_DWORD(R_EBP + 0x4) != EXPECTED_GBIMAGE2D_LOADIMAGEFILE_RETNADDR) {
        goto done;
    }
    
    // check if image already loaded, or should be automaticly loaded
    struct texture_hook_info *thinfo = &g_thinfo;
    if (thinfo->bits || test_texture_hook_noautoload(thinfo)) {
        flag = 1;
        this->Width = thinfo->width;
        this->Height = thinfo->height;
        this->BitCount = thinfo->bitcount;
        this->pBits = thinfo->bits;
    }

done:
    R_ESP += 0x10; // oldcode
    R_ESI = 1;
    if (flag) {
        // texture already loaded, should jump to processing
        M_DWORD(R_EBP - 0x4) = 2; // fake type to TGA (any number != 0 should OK)
        RETNADDR = gboffset + 0x1001E195; // jump to processing
    } else {
        // texture not loaded, should continue loading
        if (R_EAX) { // if extension is not TGA
            RETNADDR = gboffset + 0x1001E0C0; // jump to next extension checking
        } // else fallthrough the TGA processing procdure
    }
}

static MAKE_ASMPATCH(texhook_part4)
{
    int div_alpha = 1;
    struct gbImage2D *this = TOPTR(R_EBX);
    
    // check caller's address first
    if (M_DWORD(R_EBP + 0x4) != EXPECTED_GBIMAGE2D_LOADIMAGEFILE_RETNADDR) {
        goto done;
    }
    
    // fill in thinfo
    struct texture_hook_info *thinfo = &g_thinfo;
    thinfo->width = this->Width;
    thinfo->height = this->Height;
    thinfo->bitcount = this->BitCount;
    thinfo->bits = this->pBits;
    thinfo->type = TH_POST_IMAGELOAD;
    
    // run hooks
    run_texture_hooks(thinfo);
    
    // write back to gbImage2D
    this->Width = thinfo->width;
    this->Height = thinfo->height;
    this->BitCount = thinfo->bitcount;
    this->pBits = thinfo->bits;
    div_alpha = thinfo->div_alpha;
    
done:
    if (this->BitCount != 32 || !div_alpha) {
        RETNADDR = gboffset + 0x1001E321; // skip alpha div
    } // fallthrough to alpha div
}

static MAKE_ASMPATCH(texhook_part5)
{
    struct gbTexture *this = TOPTR(R_EBX);
    
    struct texture_hook_info *thinfo = &g_thinfo;
    if (thinfo->fakewidth) this->Width = thinfo->fakewidth;
    if (thinfo->fakeheight) this->Height = thinfo->fakeheight;
    
    // oldcode
    this->IsLoaded = 1;
}

void init_texture_hooks()
{
    INIT_ASMPATCH(texhook_part1, gboffset + 0x1002017C, 8, "\x8B\xF0\x33\xFF\x3B\xF7\x74\x7D");
    INIT_ASMPATCH(texhook_part2, gboffset + 0x1001E01E, 6, "\x8B\x7D\x08\x83\xC9\xFF");
    INIT_ASMPATCH(texhook_part3, gboffset + 0x1001E090, 10, "\x83\xC4\x10\xBE\x01\x00\x00\x00\x85\xC0");
    INIT_ASMPATCH(texhook_part4, gboffset + 0x1001E290, 13, "\x83\xBB\xB4\x00\x00\x00\x20\x0F\x85\x84\x00\x00\x00");
    INIT_ASMPATCH(texhook_part5, gboffset + 0x10020281, 7, "\xC7\x43\x10\x01\x00\x00\x00");
}
