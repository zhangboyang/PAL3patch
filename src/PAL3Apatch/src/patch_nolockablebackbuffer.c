#include "common.h"

// movie frame related functions
static int (WINAPI *BinkPause)(HBINK bink, int pause);

// texture
#define MF_TEX_WIDTH 1024
#define MF_TEX_HEIGHT 1024
static IDirect3DTexture9 *mf_tex = NULL;
static double mf_tex_u1, mf_tex_v1, mf_tex_u2, mf_tex_v2;
static double mf_tex_ratio;
static int mf_tex_clamp;
static int mf_bink_dstsurfacetype;
static fRECT mf_frect;

// status
static int mf_movie_playing = 0;
static int mf_movie_paused = 0;

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
        double u1, v1, u2, v2, r;
    } *p, uvdata[] = { // input custom U and V value here
        
        // opening
        { "Movie\\Pal3a.bik", 0.0, (75.0 / 600.0), 1.0, (525.0 / 600.0), -1.0 },
        
        { NULL } // EOF
    };

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
        mf_tex_u1 = mf_tex_v1 = 0.0;
        mf_tex_u2 = mf_tex_v2 = 1.0;
    }
    
    // calc movie ratio
    mf_tex_ratio = ((mf_tex_u2 - mf_tex_u1) * movie_width) / ((mf_tex_v2 - mf_tex_v1) * movie_height);
    if (p->filename) {
        if (p->r > 0) {
            mf_tex_ratio = p->r;
        }
    } else {
        if (movie_width == 800 && movie_height == 448) {
            // special fix
            mf_tex_ratio = 16.0 / 9.0;
        }
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
    get_ratio_frect(&mf_frect, &game_frect, mf_tex_ratio, TR_CENTER, TR_CENTER);
    
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
static int __stdcall BinkDoFrame_wrapper(HBINK bink)
{
    return last_BinkDoFrame_retval = BinkDoFrame(bink);
}
static int __stdcall BinkCopyToBuffer_wrapper(HBINK bink, void *dest_addr, int dest_pitch, unsigned dest_height, unsigned dest_x, unsigned dest_y, unsigned copy_flags)
{
    return last_BinkCopyToBuffer_retval = BinkCopyToBuffer(bink, dest_addr, dest_pitch, dest_height, dest_x, dest_y, copy_flags);
}

static void movie_playback_atopen(void *arg)
{
    struct game_loop_hook_data *hookarg = arg;
    
    // check hook type
    if (hookarg->type != GAMEEVENT_MOVIE_ATOPEN) return;
    
    if (!g_bink.m_hBink) return;
    
    struct gbAudioManager *pAudioMgr = SoundMgr_GetAudioMgr(SoundMgr_Inst());
    if (pAudioMgr && !xmusic) {
        BinkSetVolume(g_bink.m_hBink, floor(32768.0 * gbAudioManager_GetMusicMasterVolume(pAudioMgr)) + eps);
    } else {
        BinkSetVolume(g_bink.m_hBink, 0);
    }
    
    const char *moviefile = hookarg->data;
    
    // init vertex buffer and texture
    init_movieframe_texture(moviefile, gbBinkVideo_Width(&g_bink), gbBinkVideo_Height(&g_bink));
    
    // fill the texture with zeros
    D3DLOCKED_RECT lrc;
    IDirect3DTexture9_LockRect(mf_tex, 0, &lrc, NULL, 0);
    memset(lrc.pBits, 0, lrc.Pitch * MF_TEX_HEIGHT);
    IDirect3DTexture9_UnlockRect(mf_tex, 0);
    
    // set playing flag for cursor
    mf_movie_playing = 1;
    
    // set pause state
    mf_movie_paused = 0;
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
    int bitcount = gbGfxManager_D3D_GetBackBufferBitCount(GB_GfxMgr);
    if (mf_tex_clamp && bitcount) {
        int left = floor(MF_TEX_WIDTH * mf_tex_u1 + eps);
        int top = floor(MF_TEX_HEIGHT * mf_tex_v1 + eps);
        int right = floor(MF_TEX_WIDTH * mf_tex_u2 + eps);
        int bottom = floor(MF_TEX_HEIGHT * mf_tex_v2 + eps);
        clamp_rect(lrc.pBits, MF_TEX_WIDTH, MF_TEX_HEIGHT, bitcount, lrc.Pitch, left, top, right, bottom);
    }
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
    if (mf_tex_clamp) {
        IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    } else {
        IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
        IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
        IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_BORDERCOLOR, 0x00000000);
    }
    
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

static void movie_checkpause_hook(void *arg)
{
    int paused = *(int *) arg;
    if (paused) {
        mf_movie_paused = 1;
        if (g_bink.m_hBink) {
            BinkPause(g_bink.m_hBink, 1);
        }
    } else {
        mf_movie_paused = 0;
        if (g_bink.m_hBink) {
            BinkPause(g_bink.m_hBink, 0);
        }
    }
}
static void movie_playback_atstop(void *arg)
{
    struct game_loop_hook_data *hookarg = arg;
    
    // check hook type
    if (hookarg->type != GAMEEVENT_MOVIE_ATEND) return;
    
    // reset cursor status
    mf_movie_playing = 0;
}


static MAKE_THISCALL(int, gbBinkVideo_BinkWait_wrapper, struct gbBinkVideo *this)
{
    int ret = gbBinkVideo_BinkWait(this);
    if (ret) Sleep(1);
    return ret;
}

static void hook_gbBinkVideo()
{
    // import BinkPause
    BinkPause = TOPTR(GetProcAddress_check(GetModuleHandle_check("BINKW32.DLL"), "_BinkPause@8"));
    
    // hook member funtions    
    make_jmp(0x005252A1, gbBinkVideo_DrawFrame);
    
    // hook open operation
    add_gameloop_hook(movie_playback_atopen);
    
    // cleanup when movie loop exits
    add_gameloop_hook(movie_playback_atstop);
    
    // check state for pausing movie
    add_pauseresume_hook(movie_checkpause_hook);
    
    // hook BinkDoFrame() and BinkCopyToBuffer()
    INIT_WRAPPER_CALL(BinkDoFrame_wrapper, { 0x00525342 });
    INIT_WRAPPER_CALL(BinkCopyToBuffer_wrapper, { 0x00525363 });
    
    // hook gbBinkVideo::BinkWait() to release CPU while playing movie
    INIT_WRAPPER_CALL(gbBinkVideo_BinkWait_wrapper, { 0x00525451 });
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
static int ss_enable;
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
    if (ss_enable) {
        IDirect3DDevice9_GetFrontBufferData(gfxmgr->m_pd3dDevice, 0, ss_surface);
    } else {
        IDirect3DDevice9_ColorFill(gfxmgr->m_pd3dDevice, ss_surface, NULL, 0xFF000000);
    }
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
    get_ratio_frect(&frect, &frect, 4.0 / 3.0, TR_CENTER, TR_CENTER);
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
    switch (TOUINT(func_return_address())) {
        case 0x004ADD4E: locktype = LT_RENDERTARGET; break;
        case 0x004087A3: locktype = LT_SCREENSHOT; break;
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
    // load movie clamp settings
    mf_tex_clamp = get_int_from_configfile("clampmovie");
    
    // load screenshot settings
    ss_enable = get_int_from_configfile("saveimg");
    
    // lock/unlock hooks
    SIMPLE_PATCH(gboffset + 0x10019AF2, "\xC7\x81\x08\x07\x00\x00\x03\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x02\x00\x00\x00", 10);
    SIMPLE_PATCH(gboffset + 0x10019B22, "\xC7\x81\x08\x07\x00\x00\x01\x00\x00\x00", "\xC7\x81\x08\x07\x00\x00\x00\x00\x00\x00", 10);
    make_jmp(gboffset + 0x100189A0, LockBackBuffer);
    make_jmp(gboffset + 0x10018A20, UnlockBackBuffer);
    
    // gbBinkVideo hooks
    hook_gbBinkVideo();
    add_postd3dcreate_hook(init_movieframe_vertbuf);
}
