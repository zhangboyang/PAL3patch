#include "common.h"

// hook gbTexture_D3D::CreateForRenderTarget, clear texture after creation
static MAKE_THISCALL(int, gbTexture_D3D_CreateForRenderTarget_wrapper, struct gbTexture_D3D *this, int width, int height, enum gbPixelFmtType format)
{
    int ret = gbTexture_D3D_CreateForRenderTarget(this, width, height, format);
    if (ret) {
        fill_texture((IDirect3DTexture9 *) this->pTex, 0xFF000000);
    }
    return ret;
}

static int tex_flag;
static int tex_width, tex_height;

static void try_create_rendertarget()
{
    struct gbTexture_D3D tmp_tex;
    gbTexture_D3D_Ctor(&tmp_tex);
    enum gbPixelFmtType format = gbGfxManager_D3D_GetBackBufferFormat(GB_GfxMgr);
    tex_width = floor(get_frect_width(&game_frect) + eps);
    tex_height = floor(get_frect_height(&game_frect) + eps);
    // try to create render target texture
    // there is no need to call wrapped version (in patch_fixreset.c)
    if (gbTexture_D3D_CreateForRenderTarget(&tmp_tex, tex_width, tex_height, format)) {
        // success, use screen-size trail texture
        tex_flag = 1;
    } else {
        // failed, fallback to original size
        tex_flag = 0;
        tex_width = tex_height = 256;
    }
    gbTexture_D3D_Dtor(&tmp_tex);
    
}

static MAKE_ASMPATCH(CTrail_OnResetDevice_asmpatch)
{
    R_EAX = M_DWORD(R_ESI); // old code 
    R_ECX = R_ESI;
    
    PUSH_DWORD(tex_height);
    PUSH_DWORD(tex_width);
}

static MAKE_THISCALL(void, CTrail_Begin_wrapper, struct CTrail *this, struct gbCamera *pCam)
{
    CTrail_Begin(this, pCam);
    if (this->m_bEnable && this->m_bSupport) {
        // update camera dimention and screenquad
        if (tex_flag) {
            if (this->m_pCam) {
                gbCamera_SetDimention(this->m_pCam, tex_width, tex_height);
            }
            this->m_ScreenQuad.sv = 1.0f;
            this->m_ScreenQuad.eu = 1.0f;
            this->m_ScreenQuad.ex = tex_width;
            this->m_ScreenQuad.sy = tex_height;
        } else {
            if (this->m_pCam) {
                gbCamera_SetDimention(this->m_pCam, floor(256.0 * get_frect_width(&game_frect_sqrtex) + eps), floor(256.0 * get_frect_height(&game_frect_sqrtex) + eps));
            }
            this->m_ScreenQuad.su = game_frect_sqrtex.left;
            this->m_ScreenQuad.sv = game_frect_sqrtex.bottom;
            this->m_ScreenQuad.eu = game_frect_sqrtex.right;
            this->m_ScreenQuad.ev = game_frect_sqrtex.top;
            this->m_ScreenQuad.ex = get_frect_width(&game_frect);
            this->m_ScreenQuad.sy = get_frect_height(&game_frect);
        }
    }
}

MAKE_PATCHSET(fixtrail)
{
    // hook gbTexture_D3D::CreateForRenderTarget
    INIT_WRAPPER_VFPTR(gbTexture_D3D_CreateForRenderTarget_wrapper, gboffset + 0x100D674C);
    
    add_postd3dcreate_hook(try_create_rendertarget);
    INIT_ASMPATCH(CTrail_OnResetDevice_asmpatch, 0x004B73B9, 6, "\x8B\x06\x8B\xCE\x57\x57");
    INIT_WRAPPER_CALL(CTrail_Begin_wrapper, { 0x004ED784 });
}
