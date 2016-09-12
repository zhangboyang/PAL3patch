#include "common.h"

// game resolution
int game_width, game_height;
int game_width_43, game_height_43;
int width_shift, height_shift;
double scalefactor;



// part of the CArrayList class, from DirectX SDK C++ Sample
enum ArrayListType { AL_VALUE, AL_REFERENCE };
struct CArrayList {
    enum ArrayListType m_ArrayListType;
    void *m_pData;
    UINT m_BytesPerEntry;
    UINT m_NumEntries;
    UINT m_NumEntriesAllocated;
};
static void *CArrayList_GetPtr(struct CArrayList *this, UINT Entry)
{
    if (this->m_ArrayListType == AL_VALUE)
        return (BYTE*) this->m_pData + (Entry * this->m_BytesPerEntry);
    else
        return *(((void**) this->m_pData) + Entry);
}
static int *CArrayList_FindInt(struct CArrayList *this, int target, int *subscript)
{
    int i;
    for (i = 0; i < this->m_NumEntries; i++) {
        int *cur = CArrayList_GetPtr(this, i);
        if (*cur == target) {
            if (subscript) *subscript = i;
            return cur;
        }
    }
    if (subscript) *subscript = -1;
    return NULL;
}



// the Z-buffer patch
static int zbuf_flag;
static int zbuf_tmpint;
static MAKE_ASMPATCH(zbuf)
{
    struct CArrayList *this = TOPTR(R_ECX);
    if (zbuf_flag == INT_MAX) {
        R_EAX = TOUINT(CArrayList_GetPtr(this, this->m_NumEntries - 1));
        return;
    } else if (zbuf_flag < 0) {
        zbuf_tmpint = -zbuf_flag;
        R_EAX = TOUINT(&zbuf_tmpint);
        return;
    } else {
        int *result = NULL;
        switch (zbuf_flag) {
            case 16:
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D16, NULL);
                break;
            case 24:
                // there no need to use stencil buffer
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X8, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X4S4, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24S8, NULL);
                break;
            case 32:
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D32, NULL);
                break;
        }
        if (!result) {
            result = CArrayList_GetPtr(this, 0);
            warning("z-buffer format not supported, fallback to default configuration.");
        }
        R_EAX = TOUINT(result);
        return;
    }
}
static void patch_depth_buffer_config(const char *cfgstr)
{
    zbuf_flag = (stricmp(cfgstr, "max") == 0) ? INT_MAX : str2int(cfgstr);
    if (zbuf_flag) {
        INIT_ASMPATCH(zbuf, gboffset + 0x10019E13, 7, "\x6A\x00\xE8\x96\xD0\xFF\xFF");
        INIT_ASMPATCH(zbuf, gboffset + 0x1001A12E, 7, "\x6A\x00\xE8\x7B\xCD\xFF\xFF");
    }
}



// multisample patch
static int ms_tflag, ms_qflag;
static void set_multisample_config(struct CArrayList *tl, struct CArrayList *ql, int *tret, int *qret)
{
    if (ms_tflag < 0 || ms_qflag < 0) {
        *tret = -ms_tflag;
        *qret = -ms_qflag;
        return;
    } 
    const D3DMULTISAMPLE_TYPE msTypeArray[] = { 
        D3DMULTISAMPLE_NONE,
        D3DMULTISAMPLE_NONMASKABLE,
        D3DMULTISAMPLE_2_SAMPLES,
        D3DMULTISAMPLE_3_SAMPLES,
        D3DMULTISAMPLE_4_SAMPLES,
        D3DMULTISAMPLE_5_SAMPLES,
        D3DMULTISAMPLE_6_SAMPLES,
        D3DMULTISAMPLE_7_SAMPLES,
        D3DMULTISAMPLE_8_SAMPLES,
        D3DMULTISAMPLE_9_SAMPLES,
        D3DMULTISAMPLE_10_SAMPLES,
        D3DMULTISAMPLE_11_SAMPLES,
        D3DMULTISAMPLE_12_SAMPLES,
        D3DMULTISAMPLE_13_SAMPLES,
        D3DMULTISAMPLE_14_SAMPLES,
        D3DMULTISAMPLE_15_SAMPLES,
        D3DMULTISAMPLE_16_SAMPLES,
    };
    const UINT msTypeArrayCount = sizeof(msTypeArray) / sizeof(msTypeArray[0]);
    int id;
    if (ms_tflag == INT_MAX) {
        id = tl->m_NumEntries - 1;
        *tret = *(int *) CArrayList_GetPtr(tl, id);
    } else if (ms_tflag >= msTypeArrayCount) {
        fail("invalid multisample type configuration.");
    } else {
        int tprefer = msTypeArray[ms_tflag];
        int *pt = CArrayList_FindInt(tl, tprefer, &id);
        if (!pt) {
            warning("multisample type not supported, fallback to default configuration.");
            *tret = *qret = 0;
            return;
        }
        *tret = *pt;
    }
    int q = *(int *) CArrayList_GetPtr(ql, id);
    if (ms_qflag == INT_MAX) {
        q = q - 1;
    } else if (ms_qflag >= q) {
        warning("multisample quality %d (max %d) not supported, fallback to default configuration.", ms_qflag, q);
        q = 0;
    } else {
        q = ms_qflag;
    }
    *qret = q;
}
static MAKE_ASMPATCH(multisample_windowed)
{
    set_multisample_config(TOPTR(M_DWORD(R_EBP + 0x18)), TOPTR(M_DWORD(R_EBP + 0x1C)), TOPTR(R_EDI + 0x694), TOPTR(R_EDI + 0x698));
}
static MAKE_ASMPATCH(multisample_fullscreen)
{
    set_multisample_config(TOPTR(M_DWORD(R_ECX + 0x18)), TOPTR(M_DWORD(R_ECX + 0x1C)), TOPTR(R_ESI + 0x6CC), TOPTR(R_ESI + 0x6D0));
}
static void patch_multisample_config(const char *cfgstr)
{
    char buf[MAXLINE];
    strncpy(buf, cfgstr, sizeof(buf)); buf[sizeof(buf) - 1] = 0;
    char *ptr = strchr(buf, ',');
    if (!ptr) fail("can't parse multiplesample config string.");
    *ptr++ = 0;
    ms_tflag = (stricmp(buf, "max") == 0) ? INT_MAX : str2int(buf);
    ms_qflag = (stricmp(ptr, "max") == 0) ? INT_MAX : str2int(ptr);
    if (ms_tflag || ms_qflag) {
        INIT_ASMPATCH(multisample_windowed, gboffset + 0x10019E2E , 16, "\xC7\x87\x98\x06\x00\x00\x00\x00\x00\x00\x89\x87\x94\x06\x00\x00");
        INIT_ASMPATCH(multisample_fullscreen, gboffset + 0x1001A15B, 12, "\x89\x86\xCC\x06\x00\x00\x89\xBE\xD0\x06\x00\x00");
    }
}

// resolution patch
static char __fastcall Readn(void *this, int dummy, char *appname, char *keyname, int *ret, int defvalue)
{
    if (strcmp(keyname, "width") == 0) { *ret = game_width; return 1; }
    if (strcmp(keyname, "height") == 0) { *ret = game_height; return 1; }
    fail("invalid call to ConfigFile::Readn");
}
static void patch_resolution_config(const char *cfgstr)
{
    if (stricmp(cfgstr, "current") == 0) {
        game_width = GetSystemMetrics(SM_CXSCREEN);
        game_height = GetSystemMetrics(SM_CYSCREEN);
    } else if (sscanf(cfgstr, "%d %*[xX]%d", &game_width, &game_height) != 2) {
        warning("can't parse resolution string, fallback to default configuration.");
        game_width = GAME_WIDTH_ORG;
        game_height = GAME_HEIGHT_ORG;
    }
    
    // calc some parameters
    if (game_width * 3 >= game_height * 4) {
        game_width_43 = game_height * 4 / 3.0;
        game_height_43 = game_height;
        width_shift = (game_width - game_width_43) / 2;
        height_shift = 0;
        scalefactor = game_height / ((double) GAME_HEIGHT_ORG);
    } else {
        game_width_43 = game_width;
        game_height_43 = game_width * 0.75;
        width_shift = 0;
        height_shift = (game_height - game_height_43) / 2;
        scalefactor = game_width / ((double) GAME_WIDTH_ORG);
    }
    
    
    make_call(0x00406436, Readn);
    make_call(0x00406453, Readn);
}

MAKE_PATCHSET(graphicspatch)
{
    patch_depth_buffer_config(get_string_from_configfile("game_zbufferbits"));
    patch_multisample_config(get_string_from_configfile("game_multisample"));
    patch_resolution_config(get_string_from_configfile("game_resolution"));
}
