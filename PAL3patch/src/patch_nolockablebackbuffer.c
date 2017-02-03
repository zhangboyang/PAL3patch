#include "common.h"

//#define SAVE_MOVIESCREENSHOT_AT_STOP


// movie frame related functions

static int (*gbBinkVideo_SFLB_OpenFile)(struct gbBinkVideo *, const char *, HWND, int, int) = NULL;

// texture
#define MF_TEX_WIDTH 1024
#define MF_TEX_HEIGHT 1024
static IDirect3DTexture9 *mf_tex = NULL;
static double mf_tex_u1, mf_tex_v1, mf_tex_u2, mf_tex_v2;
static int mf_bink_dstsurfacetype;
static int mf_movie_playing = 0;
static fRECT mf_frect;

// vertex buffer
static IDirect3DVertexBuffer9 *mf_vbuf = NULL;
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

static void mf_fillvbuf_rect(struct mf_vertex_t *vbuf, const fRECT *frect, float u1, float v1, float u2, float v2)
{
    float left = frect->left;
    float top = frect->top;
    float right = frect->right;
    float bottom = frect->bottom;
    left = floor(left + eps) - 0.5f;
    right = ceil(right - eps) - 0.5f;
    top = floor(top + eps) - 0.5f;
    bottom = ceil(bottom - eps) - 0.5f;
    vbuf[0].x = vbuf[5].x = left;
    vbuf[0].y = vbuf[5].y = top;
    vbuf[0].u = vbuf[5].u = u1;
    vbuf[0].v = vbuf[5].v = v1;
    vbuf[1].x = left;
    vbuf[1].y = bottom;
    vbuf[1].u = u1;
    vbuf[1].v = v2;
    vbuf[2].x = vbuf[3].x = right;
    vbuf[2].y = vbuf[3].y = bottom;
    vbuf[2].u = vbuf[3].u = u2;
    vbuf[2].v = vbuf[3].v = v2;
    vbuf[4].x = right;
    vbuf[4].y = top;
    vbuf[4].u = u2;
    vbuf[4].v = v1;
    vbuf[0].color = vbuf[1].color = vbuf[2].color = vbuf[3].color = vbuf[4].color = vbuf[5].color = 0x00FFFFFF;
    vbuf[0].rhw = vbuf[1].rhw = vbuf[2].rhw = vbuf[3].rhw = vbuf[4].rhw = vbuf[5].rhw = 1.0f;
    vbuf[0].z = vbuf[1].z = vbuf[2].z = vbuf[3].z = vbuf[4].z = vbuf[5].z = 0.0f;
}



static void init_movieframe_vertbuf()
{
    // we need init vertbuf only once
    if (!mf_vbuf) {
        if (FAILED(IDirect3DDevice9_CreateVertexBuffer(GB_GfxMgr->m_pd3dDevice, MF_VBUF_SIZE_BYTES, 0, MF_VERTEX_FVF, D3DPOOL_MANAGED, &mf_vbuf, NULL))) {
            fail("can't create vertex buffer for movie frame.");
        }
    }
}


static void get_movie_uv(const char *filename, int movie_width, int movie_height)
{
    // calc u, v as if texture size equals to movie size
    struct {
        const char *filename;
        double u1, v1, u2, v2;
    } *p, uvdata[] = { // input custom U and V value here
        
        // opening
        { "Movie\\Pal3op.bik", 0.0, (60.0 / 480.0), 1.0, (420.0 / 480.0) },
        
        /* ending
                    id     name      top   bottom
                    ===============================
                    1     XueJian     *      *
                    2     LongKui     *      Sub
                    3     ZiXuan      *      Sub
                    4     HuaYing     *      Sub
                    5     Perfact     *      Sub
        */
        { "Movie\\END1.bik",   0.0, (75.0 / 600.0), 1.0, (525.0 / 600.0) },
        { "Movie\\END2.bik",   0.0, (75.0 / 600.0), 1.0, (575.0 / 600.0) },
        { "Movie\\END3.bik",   0.0, (75.0 / 600.0), 1.0, (575.0 / 600.0) },
        { "Movie\\END4.bik",   0.0, (75.0 / 600.0), 1.0, (575.0 / 600.0) },
        { "Movie\\END5.bik",   0.0, (75.0 / 600.0), 1.0, (585.0 / 600.0) },
        
        { NULL } // EOF
    };
    
#ifdef SAVE_MOVIESCREENSHOT_AT_STOP
    OutputDebugString(filename);
#endif

    for (p = uvdata; p->filename; p++) {
        if (stricmp(p->filename, filename) == 0) {
            mf_tex_u1 = p->u1;
            mf_tex_v1 = p->v1;
            mf_tex_u2 = p->u2;
            mf_tex_v2 = p->v2;
            break;
        }
    }
    if (!p->filename) {
        mf_tex_u1 = mf_tex_v1 = 0.0f;
        mf_tex_u2 = mf_tex_v2 = 1.0f;
    }

    // adjust to real texture coord
    if (movie_width > MF_TEX_WIDTH || movie_height > MF_TEX_HEIGHT) {
        fail("movie size is larger than texture size.");
    }
    mf_tex_u1 *= (double) movie_width / MF_TEX_WIDTH;
    mf_tex_u2 *= (double) movie_width / MF_TEX_WIDTH;
    mf_tex_v1 *= (double) movie_height / MF_TEX_HEIGHT;
    mf_tex_v2 *= (double) movie_height / MF_TEX_HEIGHT;
}

static void init_movieframe_texture(const char *filename, int movie_width, int movie_height)
{
    // create texture
    if (!mf_tex) {
        if (FAILED(IDirect3DDevice9_CreateTexture(GB_GfxMgr->m_pd3dDevice, MF_TEX_WIDTH, MF_TEX_HEIGHT, 1, 0, GB_GfxMgr->m_d3dsdBackBuffer.Format, D3DPOOL_MANAGED, &mf_tex, NULL))) {
            fail("can't create texture for movie frame.");
        }
    }
    
    // set texture information
    get_movie_uv(filename, movie_width, movie_height);
    get_ratio_frect(&mf_frect, &game_frect, (MF_TEX_WIDTH * (mf_tex_u2 - mf_tex_u1)) / (MF_TEX_HEIGHT * (mf_tex_v2 - mf_tex_v1)));
    
    // prepare target surface type for BinkVideo
    switch (GB_GfxMgr->m_d3dsdBackBuffer.Format) {
        case D3DFMT_R5G6B5:   mf_bink_dstsurfacetype = 10; break;
        case D3DFMT_X8R8G8B8: mf_bink_dstsurfacetype = 3; break;
        case D3DFMT_X1R5G5B5: mf_bink_dstsurfacetype = 9; break;
        default:              mf_bink_dstsurfacetype = 0; break;
    }
}


static int last_BinkDoFrame_retval;
static int last_BinkCopyToBuffer_retval;
static int __stdcall BinkDoFrame_wrapper(DWORD a1)
{
    return last_BinkDoFrame_retval = BinkDoFrame(a1);
}
static int __stdcall BinkCopyToBuffer_wrapper(DWORD a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5, DWORD a6, DWORD a7)
{
    return last_BinkCopyToBuffer_retval = BinkCopyToBuffer(a1, a2, a3, a4, a5, a6, a7);
}
static MAKE_THISCALL(int, gbBinkVideo_OpenFile, struct gbBinkVideo *this, const char *szFileName, HWND hWnd, int bChangeScreenMode, int nOpenFlag)
{
    int ret = gbBinkVideo_SFLB_OpenFile(this, szFileName, hWnd, bChangeScreenMode, nOpenFlag);

    // init vertex buffer and texture
    init_movieframe_texture(szFileName, gbBinkVideo_Width(this), gbBinkVideo_Height(this));
    
    // fill the texture with zeros
    D3DLOCKED_RECT lrc;
    IDirect3DTexture9_LockRect(mf_tex, 0, &lrc, NULL, 0);
    memset(lrc.pBits, 0, lrc.Pitch * MF_TEX_HEIGHT);
    IDirect3DTexture9_UnlockRect(mf_tex, 0);
    
    // set playing flag for cursor
    mf_movie_playing = 1;
    
    return ret;
}

static MAKE_THISCALL(int, gbBinkVideo_DrawFrame, struct gbBinkVideo *this)
{
    int ret;
    // check if we have inited
    if (!mf_tex || !mf_bink_dstsurfacetype) {
        return 0;
    }
    
    // check cooperative level
    gbGfxManager_D3D_EnsureCooperativeLevel(GB_GfxMgr, 1);

    // upload movie frame to texture
    D3DLOCKED_RECT lrc;
    IDirect3DTexture9_LockRect(mf_tex, 0, &lrc, NULL, 0);
    ret = gbBinkVideo_DrawFrameEx(this, lrc.pBits, lrc.Pitch, gbBinkVideo_Height(this), 0, 0, mf_bink_dstsurfacetype);
    IDirect3DTexture9_UnlockRect(mf_tex, 0);

    if (last_BinkDoFrame_retval || last_BinkCopyToBuffer_retval) {
        // the binkvideo tells us frame is skipped
        return ret;
    }
    
    // upload vertex
    struct mf_vertex_t *vbuf;
    IDirect3DVertexBuffer9_Lock(mf_vbuf, 0, 0, (void *) &vbuf, 0);
    mf_fillvbuf_rect(vbuf, &mf_frect, mf_tex_u1, mf_tex_v1, mf_tex_u2, mf_tex_v2);
    IDirect3DVertexBuffer9_Unlock(mf_vbuf);
    
    // prepare d3d state
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ZENABLE, TRUE);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_SHADEMODE, D3DSHADE_FLAT);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_LIGHTING, FALSE);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_BORDERCOLOR, 0x00000000);
    
    IDirect3DDevice9_SetTexture(GB_GfxMgr->m_pd3dDevice, 0, (void *) mf_tex);
    
    // clear surface
    IDirect3DDevice9_Clear(GB_GfxMgr->m_pd3dDevice, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
    
    // draw
    IDirect3DDevice9_BeginScene(GB_GfxMgr->m_pd3dDevice);
    IDirect3DDevice9_SetFVF(GB_GfxMgr->m_pd3dDevice, MF_VERTEX_FVF);
    IDirect3DDevice9_SetStreamSource(GB_GfxMgr->m_pd3dDevice, 0, mf_vbuf, 0, MF_VERTEX_SIZE);
    IDirect3DDevice9_DrawPrimitive(GB_GfxMgr->m_pd3dDevice, D3DPT_TRIANGLELIST, 0, MF_VBUF_TRANGLE_COUNT);
    
    // end scene
    call_preendscene_hooks();
    IDirect3DDevice9_EndScene(GB_GfxMgr->m_pd3dDevice);
    
    // present
    IDirect3DDevice9_Present(GB_GfxMgr->m_pd3dDevice, NULL, NULL, NULL, NULL);
    call_postpresent_hooks();
    
    return ret;
}

static void movie_playback_atstop()
{
    // check hook type
    if (gameloop_hookflag != GAMELOOP_MOVIE_ATEXIT) return;
    
    // reset cursor status
    mf_movie_playing = 0;
    
#ifdef SAVE_MOVIESCREENSHOT_AT_STOP
    char fnbuf[MAXLINE];
    snprintf(fnbuf, sizeof(fnbuf), "movie_%u.bmp", (unsigned) time(NULL));
    OutputDebugString(fnbuf);
    D3DXSaveTextureToFileA(fnbuf, D3DXIFF_BMP, (void *) mf_tex, NULL);
#endif
}


static int wm_setcursor_hook()
{
    int m_curfrmid = *(int *) 0x00DBD898;
    // if the condition is TRUE, UICursor::Show(1) will be called
    return m_curfrmid != 9 && !mf_movie_playing;
}

static MAKE_THISCALL(int, gbBinkVideo_BinkWait_wrapper, struct gbBinkVideo *this)
{
    int ret = gbBinkVideo_BinkWait(this);
    if (ret) Sleep(1);
    return ret;
}

static void hook_gbBinkVideo()
{
    // hook member funtions
    gbBinkVideo_SFLB_OpenFile = get_branch_jtarget(0x0053C455, 0xE8);
    make_jmp(0x0053C440, gbBinkVideo_OpenFile);
    make_jmp(0x0053C470, gbBinkVideo_DrawFrame);
    
    // cleanup when movie loop exits
    add_gameloop_hook(movie_playback_atstop);
    
    // hook WM_SETCURSOR
    make_call(0x00404DFF, wm_setcursor_hook);
    SIMPLE_PATCH(0x00404E04, "\x00\x09", "\x85\xC0", 2);
    
    // hook BinkDoFrame() and BinkCopyToBuffer()
    make_branch(0x0053C544, 0xE8, BinkDoFrame_wrapper, 6);
    make_branch(0x0053C571, 0xE8, BinkCopyToBuffer_wrapper, 6);
    
    // hook gbBinkVideo::BinkWait() to release CPU while playing movie
    INIT_WRAPPER_CALL(gbBinkVideo_BinkWait_wrapper, { 0x0053C672 });
}






// define locktypes
enum locktype_t {
    LT_UNKNOWN, LT_RENDERTARGET, LT_SCREENSHOT,
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


// screenshot hooks
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
    get_ratio_frect(&frect, &frect, 4.0 / 3.0);
    set_rect_frect(&GameRect, &frect);
        
    if (GameRect.left < 0) GameRect.left = 0;
    if (GameRect.top < 0) GameRect.top = 0;
    if (GameRect.left > surface_width - 1) GameRect.left = surface_width - 1;
    if (GameRect.top > surface_height - 1) GameRect.top = surface_height - 1;
    
    if (GameRect.right < 0) GameRect.right = 0;
    if (GameRect.bottom < 0) GameRect.bottom = 0;
    if (GameRect.right > surface_width - 1) GameRect.right = surface_width - 1;
    if (GameRect.bottom > surface_height - 1) GameRect.bottom = surface_height - 1;
    
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

// rewriteen class member functions
static MAKE_THISCALL(int, LockBackBuffer, struct gbGfxManager_D3D *this, struct gbSurfaceDesc *surface, int flags)
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
        case 0x004BDB7B: locktype = LT_RENDERTARGET; break;
        case 0x00406DA2: locktype = LT_SCREENSHOT; break;
        default: locktype = LT_UNKNOWN; break;
    }
    
    // do hook
    switch (locktype) {
        case LT_RENDERTARGET: break; // do nothing
        case LT_SCREENSHOT: before_screenshot(surface); break;
        default: before_unknownlocktype(surface); break;
    }
    return 1;
}

static MAKE_THISCALL(void, UnlockBackBuffer, struct gbGfxManager_D3D *this)
{
    switch (locktype) {
        case LT_RENDERTARGET: break; // do nothing
        case LT_SCREENSHOT: after_screenshot(); break;
        default: after_unknownlocktype(); break;
    }
}



MAKE_PATCHSET(nolockablebackbuffer)
{
    // lock/unlock hooks
    SIMPLE_PATCH(gboffset + 0x1001A20F, "\xC7\x81\x08\x07\x00\x00\x03\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x02\x00\x00\x00", 10);
    SIMPLE_PATCH(gboffset + 0x1001A23F, "\xC7\x81\x08\x07\x00\x00\x01\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x00\x00\x00\x00", 10);
    make_jmp(gboffset + 0x10018F40, LockBackBuffer);
    make_jmp(gboffset + 0x10019030, UnlockBackBuffer);
    
    // gbBinkVideo hooks
    hook_gbBinkVideo();
    add_postd3dcreate_hook(init_movieframe_vertbuf);
}
