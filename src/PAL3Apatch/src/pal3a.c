#include "common.h"

// here are some common functions related to GBENGINE.DLL or PAL3.EXE

// this is my own method! not exists in original GBENGINE
enum gbPixelFmtType gbGfxManager_D3D_GetBackBufferFormat(struct gbGfxManager_D3D *this)
{
    enum gbPixelFmtType type;
    switch (this->m_d3dsdBackBuffer.Format) {
        case D3DFMT_A8R8G8B8: type = GB_PFT_A8R8G8B8; break;
        case D3DFMT_R5G6B5:   type = GB_PFT_R5G6B5;   break;
        case D3DFMT_A1R5G5B5: type = GB_PFT_A1R5G5B5; break;
        case D3DFMT_A4R4G4B4: type = GB_PFT_A4R4G4B4; break;
        case D3DFMT_X8R8G8B8: type = GB_PFT_X8R8G8B8; break;
        case D3DFMT_X1R5G5B5: type = GB_PFT_X1R5G5B5; break;
        case D3DFMT_X4R4G4B4: type = GB_PFT_X4R4G4B4; break;
        default:              type = GB_PFT_R8G8B8;   break;
    }
    return type;
}

// this is my own method! not exists in original GBENGINE
int gbGfxManager_D3D_GetBackBufferBitCount(struct gbGfxManager_D3D *this)
{
    switch (this->m_d3dsdBackBuffer.Format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
            return 32;
        case D3DFMT_A1R5G5B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_R5G6B5:
            return 16;
        default:
            return 0;
    }
}

// make sure cooperative level is D3D_OK
// this is my own method! not exists in original GBENGINE
void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus)
{
    if (requirefocus && !PAL3_s_bActive) {
        while (1) {
            MSG msg;
            // we must process message queue here
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    PostQuitMessage(msg.wParam);
                    return;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (PAL3_s_bActive) {
                break;
            }
            
            set_pauseresume(1);
            call_gameloop_hooks(GAMELOOP_SLEEP, NULL);
            Sleep(100);
        }
    }
    if (IDirect3DDevice9_TestCooperativeLevel(this->m_pd3dDevice) != D3D_OK) {
        this->m_bDeviceLost = 1;
        while (1) {
            MSG msg;
            // we must process message queue here
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    PostQuitMessage(msg.wParam);
                    return;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (IDirect3DDevice9_TestCooperativeLevel(this->m_pd3dDevice) == D3DERR_DEVICENOTRESET) {
                break;
            }
            
            set_pauseresume(1);
            call_gameloop_hooks(GAMELOOP_DEVICELOST, NULL);
            Sleep(100);
        }
        if (gbGfxManager_D3D_Reset3DEnvironment(this) < 0) {
            fail("Reset3DEnvironment error!");
        }
        this->m_bDeviceLost = 0;
    }
    
    try_refresh_clipcursor();
    set_pauseresume(0);
}
void ensure_cooperative_level(int requirefocus)
{
    gbGfxManager_D3D_EnsureCooperativeLevel(GB_GfxMgr, requirefocus);
}


// load image bits from file in memory, using D3DX
// will allocate memory from given allocator
void *load_image_bits(void *filedata, unsigned filelen, int *width, int *height, int *bitcount, const struct memory_allocator *mem_allocator)
{
    D3DXIMAGE_INFO img_info;
    D3DLOCKED_RECT lrc;
    IDirect3DSurface9 *suf = NULL;
    void *bits = NULL;
    
    if (FAILED(myD3DXGetImageInfoFromFileInMemory(filedata, filelen, &img_info))) goto fail;
    *width = img_info.Width;
    *height = img_info.Height;
    *bitcount = 32; // FIXME: detect if alpha exists
    D3DFORMAT target_format;
    switch (*bitcount) {
        case 32: target_format = D3DFMT_A8R8G8B8; break;
        case 24: target_format = D3DFMT_R8G8B8; break;
        default: fail("unknown bitcount %d.", *bitcount);
    }
    if (FAILED(IDirect3DDevice9_CreateOffscreenPlainSurface(GB_GfxMgr->m_pd3dDevice, img_info.Width, img_info.Height, target_format, D3DPOOL_SCRATCH, &suf, NULL))) {
        suf = NULL;
        goto fail;
    }
    if (FAILED(myD3DXLoadSurfaceFromFileInMemory(suf, NULL, NULL, filedata, filelen, NULL, D3DX_DEFAULT, 0, NULL))) goto fail;
    if (FAILED(IDirect3DSurface9_LockRect(suf, &lrc, NULL, 0))) goto fail;
    bits = mem_allocator->malloc(img_info.Width * img_info.Height * (*bitcount / 8));
    int i;
    for (i = 0; i < (int) img_info.Height; i++) {
        memcpy(PTRADD(bits, i * img_info.Width * (*bitcount / 8)), PTRADD(lrc.pBits, i * lrc.Pitch), img_info.Width * (*bitcount / 8));
    }
    /* // fill random color, for debug purpose
    for (i = 0; i < (*height) * (*width); i++) {
        unsigned char *c = bits + i * (*bitcount / 8);
        c[0] = rand(); c[1] = rand(); c[2] = rand();
    }*/
    IDirect3DSurface9_UnlockRect(suf);
    IDirect3DSurface9_Release(suf);
    return bits;
fail:
    if (suf) IDirect3DSurface9_Release(suf);
    mem_allocator->free(bits);
    return NULL;
}

// load a file from current gbVFileSystem
// will allocate memory from given allocator
void *vfs_readfile(const char *filepath, unsigned *length, const struct memory_allocator *mem_allocator)
{
    struct gbVFile *fp = gbVFileSystem_OpenFile(g_pVFileSys, filepath, 0x201u);
    if (!fp) return NULL;
    *length = gbVFileSystem_GetFileSize(g_pVFileSys, fp);
    void *data = mem_allocator->malloc(*length);
    gbVFileSystem_Read(g_pVFileSys, data, *length, fp);
    gbVFileSystem_CloseFile(g_pVFileSys, fp);
    return data;
}

// get CPK name from current gbVFileSystem
// may return pointer to static buffer, or empty string when no CPK is used
const char *vfs_cpkname()
{
    static char cpknamebuf[MAXLINE];
    const char *cpkname = g_pVFileSys->rtDirectory;
    if (cpkname && (str_endswith(cpkname, "\\") || str_endswith(cpkname, "/"))) {
        // cpkname is illegal, possiblly nocpk is enabled
        // try convert to legal cpkname
        snprintf(cpknamebuf, sizeof(cpknamebuf), "%.*s.cpk", strlen(cpkname) - 1, cpkname);
        cpkname = cpknamebuf;
    }
    
    // cut cpkname
    if (cpkname) {
        char *separator = strrchr(cpkname, '\\');
        if (separator) cpkname = separator + 1;
    }
    
    return cpkname ? cpkname : "";
}

void make_area_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom)
{
    assert(bitcount == 32);
    int i, j;
    for (i = top; i < bottom; i++) {
        unsigned char *line = PTRADD(bits, i * pitch);
        for (j = left; j < right; j++) {
            line[4 * j + 3] = 0x00;
        }
    }
}

void make_border_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom)
{
    assert(bitcount == 32);
    make_area_transparent(bits, width, height, bitcount, pitch, 0, 0, width, top);
    make_area_transparent(bits, width, height, bitcount, pitch, 0, bottom, width, height);
    make_area_transparent(bits, width, height, bitcount, pitch, 0, top, left, bottom);
    make_area_transparent(bits, width, height, bitcount, pitch, right, top, width, bottom);
}

int is_area_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom)
{
    if (bitcount == 32) {
        int i, j;
        for (i = top; i < bottom; i++) {
            unsigned char *line = PTRADD(bits, i * pitch);
            for (j = left; j < right; j++) {
                if (line[4 * j + 3] != 0x00) {
                    return 0;
                }
            }
        }
        return 1;
    } else {
        return 0;
    }
}

void get_solid_area(void *bits, int width, int height, int bitcount, int pitch, int *left, int *top, int *right, int *bottom)
{
    *left = *top = 0;
    *right = width;
    *bottom = height;
    
    if (bitcount == 32) { // image have alpha channel
        while (*top < *bottom - 1 && is_area_transparent(bits, width, height, bitcount, pitch, *left, *top, *right, *top + 1)) (*top)++;
        while (*top < *bottom - 1 && is_area_transparent(bits, width, height, bitcount, pitch, *left, *bottom - 1, *right, *bottom)) (*bottom)--;
        while (*left < *right - 1 && is_area_transparent(bits, width, height, bitcount, pitch, *left, *top, *left + 1, *bottom)) (*left)++;
        while (*left < *right - 1 && is_area_transparent(bits, width, height, bitcount, pitch, *right - 1, *top, *right, *bottom)) (*right)--;
    }
}

void clamp_rect(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom)
{
    int i, j;
    int bytecount = bitcount / 8;
    for (i = 0; i < height; i++) {
        char *line = PTRADD(bits, i * pitch);
        for (j = 0; j < left; j++) {
            memcpy(line + j * bytecount, line + left * bytecount, bytecount);
        }
        for (j = right; j < width; j++) {
            memcpy(line + j * bytecount, line + (right - 1) * bytecount, bytecount);
        }
    }
    for (i = 0; i < top; i++) {
        memcpy(PTRADD(bits, i * pitch), PTRADD(bits, top * pitch), width * bytecount);
    }
    for (i = bottom; i < height; i++) {
        memcpy(PTRADD(bits, i * pitch), PTRADD(bits, (bottom - 1) * pitch), width * bytecount);
    }
}

void copy_bits(void *dst, int dst_pitch, int dst_x, int dst_y, void *src, int src_pitch, int src_x, int src_y, int width, int height, int bitcount)
{
    dst = PTRADD(dst, dst_pitch * dst_y + dst_x * (bitcount / 8));
    src = PTRADD(src, src_pitch * src_y + src_x * (bitcount / 8));
    int copypitch = width * (bitcount / 8);
    int i;
    for (i = 0; i < height; i++) {
        memcpy(dst, src, copypitch);
        dst = PTRADD(dst, dst_pitch);
        src = PTRADD(src, src_pitch);
    }
}

// setup matrices for d3dxfont manually
void set_d3dxfont_matrices(IDirect3DDevice9 *pd3dDevice)
{
    float world[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };
    float view[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f },
    };
    float projection[4][4] = {
		{ 2.0f / get_frect_width(&game_frect), 0.0f, 0.0f, 0.0f },
		{ 0.0f, -2.0f / get_frect_height(&game_frect), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
    };
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_WORLD, (D3DMATRIX *) &world);
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_VIEW, (D3DMATRIX *) &view);
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_PROJECTION, (D3DMATRIX *) &projection);
}


// fill texture with given color
static VOID WINAPI fill_texture_callback(D3DXVECTOR4 *pOut, const D3DXVECTOR2 *pTexCoord, const D3DXVECTOR2 *pTexelSize, LPVOID pData)
{
   *pOut = *(D3DXVECTOR4 *) pData;
}
void fill_texture(IDirect3DTexture9 *tex, D3DCOLOR color)
{
    float r = (color & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = ((color >> 16) & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    D3DXVECTOR4 vcolor = (D3DXVECTOR4) { r, g, b, a };
    myD3DXFillTexture(tex, fill_texture_callback, &vcolor);
}


// read raw compressed file from cpk
struct rawcpk {
    FILE *fp;
    struct CPKHeader hdr;
    struct CPKTable tbl[0x8000];
};
struct rawcpk *rawcpk_open(const char *cpkpath)
{
    struct rawcpk *rcpk = malloc(sizeof(struct rawcpk));
    if (!rcpk) return NULL;
    memset(rcpk, 0, sizeof(struct rawcpk));
    rcpk->fp = fopen(cpkpath, "rb");
    if (!rcpk->fp) goto fail;
    if (fread(&rcpk->hdr, sizeof(rcpk->hdr), 1, rcpk->fp) != 1) goto fail;
    if (rcpk->hdr.dwMaxFileNum > sizeof(rcpk->tbl) / sizeof(rcpk->tbl[0])) goto fail;
    if (rcpk->hdr.dwValidTableNum > rcpk->hdr.dwMaxFileNum) goto fail;
    if (fread(rcpk->tbl, sizeof(struct CPKTable), rcpk->hdr.dwValidTableNum, rcpk->fp) != rcpk->hdr.dwValidTableNum) goto fail;
    return rcpk;
fail:
    if (rcpk->fp) fclose(rcpk->fp);
    free(rcpk);
    return NULL;
}
void rawcpk_close(struct rawcpk *rcpk)
{
    fclose(rcpk->fp);
    free(rcpk);
}
void *rawcpk_read(struct rawcpk *rcpk, unsigned keycrc, unsigned *packedsize)
{
    struct CPKTable *cpkitem;
    int left, right, mid;
    left = 0;
    right = rcpk->hdr.dwValidTableNum;
    while (1) {
        if (left == right) return NULL;
        mid = left + (right - left) / 2;
        cpkitem = &rcpk->tbl[mid];
        if (cpkitem->dwCRC == keycrc && (cpkitem->dwFlag & 0x1) && !(cpkitem->dwFlag & 0x10)) {
            break;
        }
        if (left + 1 == right) return NULL;
        if (keycrc >= cpkitem->dwCRC) {
            left = mid;
        } else {
            right = mid;
        }
    }
    void *rawdata = NULL;
    if (fseek(rcpk->fp, cpkitem->dwStartPos, SEEK_SET) != 0) goto fail;
    rawdata = malloc(cpkitem->dwPackedSize);
    if (!rawdata) goto fail;
    if (fread(rawdata, 1, cpkitem->dwPackedSize, rcpk->fp) != cpkitem->dwPackedSize) goto fail;
    *packedsize = cpkitem->dwPackedSize;
    return rawdata;
fail:
    free(rawdata);
    return NULL;
}
char *rawcpk_hash(struct rawcpk *rcpk, unsigned keycrc, char *buf)
{
    unsigned packedsize;
    void *rawdata = rawcpk_read(rcpk, keycrc, &packedsize);
    if (!rawdata) return NULL;
	unsigned char digest[20];
	SHA1_CTX ctx;
	SHA1Init(&ctx);
	SHA1Update(&ctx, rawdata, packedsize);
	SHA1Final(digest, &ctx);
    free(rawdata);
	unsigned i;
	for (i = 0; i < sizeof(digest); i++) {
		sprintf(buf + i * 2, "%02x", (unsigned) digest[i]);
	}
    return buf;
}
