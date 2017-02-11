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

// make sure cooperative level is D3D_OK
// this is my own method! not exists in original GBENGINE
void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus)
{
    if (requirefocus && !PAL3_s_bActive) {
        while (1) {
            MSG msg;
            // we must process message queue here
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return;
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
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return;
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
        set_pauseresume(0);
    }
}

// load image bits from file in memory, using D3DX
// will allocate memory from given allocator
void *load_image_bits(void *filedata, unsigned filelen, int *width, int *height, int *bitcount, struct memory_allocator *mem_allocator)
{
    D3DXIMAGE_INFO img_info;
    D3DLOCKED_RECT lrc;
    IDirect3DSurface9 *suf = NULL;
    void *bits = NULL;
    
    if (FAILED(D3DXFUNC(D3DXGetImageInfoFromFileInMemory)(filedata, filelen, &img_info))) goto fail;
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
    if (FAILED(D3DXFUNC(D3DXLoadSurfaceFromFileInMemory)(suf, NULL, NULL, filedata, filelen, NULL, D3DX_DEFAULT, 0, NULL))) goto fail;
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
void *vfs_readfile(const char *filepath, unsigned *length, struct memory_allocator *mem_allocator)
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
    if (cpkname && (str_iendwith(cpkname, "\\") || str_iendwith(cpkname, "/"))) {
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
