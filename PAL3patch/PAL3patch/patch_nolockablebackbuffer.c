#include "common.h"




enum locktype_t {
    LT_UNKNOWN, LT_RENDERTARGET, LT_SCREENSHOT, LT_MOVIEFRAME,
};
static struct gbGfxManager_D3D *gfxmgr;
static enum locktype_t locktype;



// provide a dummy memory region for unknown lock operation
static void *pbits_ptr;
static void before_unknownlocktype(struct gbSurfaceDesc *surface)
{
    surface->pitch = surface->width * 4;
    surface->pbits = pbits_ptr = malloc(surface->height * surface->pitch);
}
static void after_unknownlocktype()
{
    free(pbits_ptr);
}



static IDirect3DSurface9 *ss_surface;
static void before_screenshot(struct gbSurfaceDesc *surface)
{
    int surface_width, surface_height;
    if (gfxmgr->DrvInfo.fullscreen) {
        surface_width = game_width;
        surface_height = game_height;
    } else {
        D3DDISPLAYMODE CurDisplayMode;
        IDirect3D9_GetAdapterDisplayMode(gfxmgr->m_pD3D, D3DADAPTER_DEFAULT, &CurDisplayMode);
        surface_width = CurDisplayMode.Width;
        surface_height = CurDisplayMode.Height;
    }
    
    IDirect3DDevice9_CreateOffscreenPlainSurface(gfxmgr->m_pd3dDevice, surface_width, surface_height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, (void *) &ss_surface, NULL);
    IDirect3DDevice9_GetFrontBufferData(gfxmgr->m_pd3dDevice, 0, ss_surface);
    RECT GameRect;
    if (gfxmgr->DrvInfo.fullscreen) {
        GameRect.left = GameRect.top = 0;
        GameRect.right = game_width;
        GameRect.bottom = game_height;
    } else {
        POINT Point;
        Point.x = Point.y = 0;
        ClientToScreen(gfxmgr->DrvInfo.hgfxwnd, &Point);
        GameRect.left = Point.x;
        GameRect.top = Point.y;
        GameRect.right = GameRect.left + game_width;
        GameRect.bottom = GameRect.top + game_height;
    }

    fRECT frect;
    set_frect_rect(&frect, &GameRect);
    get_43_frect(&frect, &frect);
    set_rect_frect(&GameRect, &frect);
        
    if (GameRect.left < 0) GameRect.left = 0;
    if (GameRect.top < 0) GameRect.top = 0;
    if (GameRect.right > surface_width) GameRect.right = surface_width;
    if (GameRect.bottom > surface_height) GameRect.bottom = surface_height;
    if (GameRect.right <= GameRect.left) GameRect.right = GameRect.left + 1;
    if (GameRect.bottom <= GameRect.top) GameRect.bottom = GameRect.top + 1;
    
    D3DLOCKED_RECT LockedRect;
    IDirect3DSurface9_LockRect(ss_surface, &LockedRect, &GameRect, 0);
    surface->width = GameRect.right - GameRect.left;
    surface->height = GameRect.bottom - GameRect.top;
    surface->pitch = LockedRect.Pitch;
    surface->pbits = LockedRect.pBits;
    surface->format = GB_PFT_X8R8G8B8; // PAL3 doesn't regconize GB_PFT_A8R8G8B8
}
static void after_screenshot()
{
    IDirect3DSurface9_UnlockRect(ss_surface);
    IDirect3DSurface9_Release(ss_surface);
}




// movie frame related functions
static int mf_init_flag = 0;
static IDirect3DTexture9 *mf_tex;
static IDirect3DVertexBuffer9 *mf_vbuf;
struct mf_vertex_t {
    float x, y, z, rhw;
    DWORD color;
    float u, v;
};
#define MF_VERTEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define MF_VERTEX_SIZE (sizeof(struct mf_vertex_t))
#define MF_VBUF_TRANGLE_COUNT 2
#define MF_VBUF_SIZE (MF_VBUF_TRANGLE_COUNT * 3)
#define MF_VBUF_SIZE_BYTES (MF_VERTEX_SIZE * MF_VBUF_SIZE)
static void init_movieframe(struct gbSurfaceDesc *surface)
{
    if (FAILED(IDirect3DDevice9_CreateTexture(gfxmgr->m_pd3dDevice, surface->width, surface->height, 1, 0, gfxmgr->m_d3dsdBackBuffer.Format, D3DPOOL_MANAGED, &mf_tex, NULL))) {
        fail("can't create texture for movie frame.");
    }
    if (FAILED(IDirect3DDevice9_CreateVertexBuffer(gfxmgr->m_pd3dDevice, MF_VBUF_SIZE_BYTES, 0, MF_VERTEX_FVF, D3DPOOL_MANAGED, &mf_vbuf, NULL))) {
        fail("can't create vertex buffer for movie frame.");
    }
    mf_init_flag = 1;
}
static void before_movieframe(struct gbSurfaceDesc *surface)
{
    surface->width = GAME_WIDTH_ORG;
    surface->height = GAME_HEIGHT_ORG;
    if (!mf_init_flag) init_movieframe(surface);
    
    gbGfxManager_D3D_EnsureCooperativeLevel(gfxmgr, 1);
    // NOTE: we must set locktype to LT_MOVIEFRAME here, because:
    // if device lost happends:
    //   gbGfxManager_D3D::D3D_Reset3DEnvironment() will call:
    //      RenderTarget::OnResetDevice() will call:
    //         RenderTarget::Create() will call:
    //            gbGfxManager_D3D::LockBackBuffer()
    //                => set locktype=LT_RENDERTARGET (OVERWRITE OUR LOCKTYPE!)
    locktype = LT_MOVIEFRAME;
    
    gfxmgr->m_bShowCursor = 0;
    IDirect3DDevice9_ShowCursor(gfxmgr->m_pd3dDevice, FALSE);
    IDirect3DDevice9_Clear(gfxmgr->m_pd3dDevice, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
    D3DLOCKED_RECT LockedRect;
    IDirect3DTexture9_LockRect(mf_tex, 0, &LockedRect, NULL, 0);
    surface->pitch = LockedRect.Pitch;
    surface->pbits = LockedRect.pBits;
    memset(surface->pbits, 0, surface->height * surface->pitch);
}
static void mf_fillvbuf_rect(struct mf_vertex_t *vbuf, const fRECT *frect)
{
    float left = frect->left;
    float top = frect->top;
    float right = frect->right;
    float bottom = frect->bottom;
    left = floor(left) - 0.5f;
    right = floor(right) - 0.5f;
    top = floor(top) - 0.5f;
    bottom = floor(bottom) - 0.5f;
    memset(vbuf, 0, MF_VBUF_SIZE_BYTES);
    vbuf[0].x = vbuf[5].x = left;
    vbuf[0].y = vbuf[5].y = top;
    vbuf[1].x = left;
    vbuf[1].y = bottom;
    vbuf[1].v = 1.0f;
    vbuf[2].x = vbuf[3].x = right;
    vbuf[2].y = vbuf[3].y = bottom;
    vbuf[2].u = vbuf[3].u = 1.0f;
    vbuf[2].v = vbuf[3].v = 1.0f;
    vbuf[4].x = right;
    vbuf[4].y = top;
    vbuf[4].u = 1.0f;
    vbuf[0].color = vbuf[1].color = vbuf[2].color = vbuf[3].color = vbuf[4].color = vbuf[5].color = 0x00FFFFFF;
    vbuf[0].rhw = vbuf[1].rhw = vbuf[2].rhw = vbuf[3].rhw = vbuf[4].rhw = vbuf[5].rhw = 1.0f;
}
static void after_movieframe()
{
    struct mf_vertex_t *vbuf;
    IDirect3DTexture9_UnlockRect(mf_tex, 0);
    
    IDirect3DVertexBuffer9_Lock(mf_vbuf, 0, 0, (void *) &vbuf, 0);
    mf_fillvbuf_rect(vbuf, &game_frect_43);
    IDirect3DVertexBuffer9_Unlock(mf_vbuf);
    
    IDirect3DDevice9_SetRenderState(gfxmgr->m_pd3dDevice, D3DRS_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice9_SetRenderState(gfxmgr->m_pd3dDevice, D3DRS_ZENABLE, TRUE);
    IDirect3DDevice9_SetTextureStageState(gfxmgr->m_pd3dDevice, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    IDirect3DDevice9_SetTextureStageState(gfxmgr->m_pd3dDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(gfxmgr->m_pd3dDevice, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    IDirect3DDevice9_SetTextureStageState(gfxmgr->m_pd3dDevice, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    IDirect3DDevice9_SetRenderState(gfxmgr->m_pd3dDevice, D3DRS_LIGHTING, FALSE);
    IDirect3DDevice9_SetSamplerState(gfxmgr->m_pd3dDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(gfxmgr->m_pd3dDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(gfxmgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    IDirect3DDevice9_SetSamplerState(gfxmgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    IDirect3DDevice9_SetSamplerState(gfxmgr->m_pd3dDevice, 0, D3DSAMP_BORDERCOLOR, 0x00000000);
    IDirect3DDevice9_SetTexture(gfxmgr->m_pd3dDevice, 0, (void *) mf_tex);
    
    IDirect3DDevice9_BeginScene(gfxmgr->m_pd3dDevice);
    IDirect3DDevice9_SetFVF(gfxmgr->m_pd3dDevice, MF_VERTEX_FVF);
    IDirect3DDevice9_SetStreamSource(gfxmgr->m_pd3dDevice, 0, mf_vbuf, 0, MF_VERTEX_SIZE);
    IDirect3DDevice9_DrawPrimitive(gfxmgr->m_pd3dDevice, D3DPT_TRIANGLELIST, 0, MF_VBUF_TRANGLE_COUNT);
    IDirect3DDevice9_EndScene(gfxmgr->m_pd3dDevice);
    // PAL3.EXE calls Present(), we don't need to call it here
    //IDirect3DDevice9_Present(gfxmgr->m_pd3dDevice, NULL, NULL, NULL, NULL);
}







// rewriteen class member functions
static __fastcall int LockBackBuffer(struct gbGfxManager_D3D *this, int dummy, struct gbSurfaceDesc *surface, int flags)
{
    // fill the surface structure
    // note: surface->pitch and surface->pbits will be filled later
    //       and these paramters may be overwrited by hook functions

    surface->width = this->m_d3dsdBackBuffer.Width;
    surface->height = this->m_d3dsdBackBuffer.Height;
    surface->format = gbGfxManager_D3D_GetBackBufferFormat(this);
    surface->pitch = 0;
    surface->pbits = NULL;
    
    // save parameters
    gfxmgr = this;
    switch (TOUINT(__builtin_return_address(0))) {
        case 0x004BDB7B: locktype = LT_RENDERTARGET;
            // NOTE: must do nothing, see before_movieframe() for details
            break;
        case 0x00406DA2: locktype = LT_SCREENSHOT; break;
        case 0x0053C49D: locktype = LT_MOVIEFRAME; break;
        default: locktype = LT_UNKNOWN; break;
    }
    
    // do hook
    switch (locktype) {
        case LT_RENDERTARGET:
            // NOTE: must do nothing, see before_movieframe() for details
            break;
        case LT_SCREENSHOT: before_screenshot(surface); break;
        case LT_MOVIEFRAME: before_movieframe(surface); break;
        default: before_unknownlocktype(surface); break;
    }
    return 1;
}

static __fastcall void UnlockBackBuffer(struct gbGfxManager_D3D *this, int dummy)
{
    switch (locktype) {
        case LT_RENDERTARGET:
            // NOTE: must do nothing, see before_movieframe() for details
            break;
        case LT_SCREENSHOT: after_screenshot(); break;
        case LT_MOVIEFRAME: after_movieframe(); break;
        default: after_unknownlocktype(); break;
    }
}

MAKE_PATCHSET(nolockablebackbuffer)
{
    SIMPLE_PATCH(gboffset + 0x1001A20F, "\xC7\x81\x08\x07\x00\x00\x03\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x02\x00\x00\x00", 10);
    SIMPLE_PATCH(gboffset + 0x1001A23F, "\xC7\x81\x08\x07\x00\x00\x01\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x00\x00\x00\x00", 10);
    make_jmp(gboffset + 0x10018F40, LockBackBuffer);
    make_jmp(gboffset + 0x10019030, UnlockBackBuffer);
}
