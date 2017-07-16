#include "common.h"


struct fvf_xyztex2 {
    float x, y, z;
    float u1, v1;
    float u2, v2;
};

static MAKE_ASMPATCH(fixtexcoord_mode0_part1)
{
    struct fvf_xyztex2 *v = TOPTR(R_ECX);
    IDirect3DVertexBuffer9 *vbuf = TOPTR(R_EAX);
    
    if (!RenderTarget_Inst()->m_iMode) {
        // fix tex2 (rt texture) coord
        // only need fix when using RenderTarget::m_Texture
        int i;
        int n = 4;
        for (i = 0; i < n; i++) {
            v[i].u2 *= game_frect_sqrtex.right;
            v[i].v2 *= game_frect_sqrtex.bottom;
        }
    }
    
    // oldcode
    IDirect3DVertexBuffer9_Unlock(vbuf);
}

static MAKE_ASMPATCH(fixtexcoord_mode0_part2)
{
    struct fvf_xyztex2 *v = TOPTR(R_EDX);
    struct UnderWater *this = TOPTR(R_ESI);
    IDirect3DVertexBuffer9 *vbuf = this->m_pVB;
    
    // fix tex1 (bumpmap texture) coord
    float t = this->m_fTime * 0.5f;
    float left = t;
    float right = left + 1.0f;
    float top = 0.0f;
    float bottom = 1.0f;
    v[0].u1 = left;
    v[0].v1 = bottom;
    v[1].u1 = left;
    v[1].v1 = top;
    v[2].u1 = right;
    v[2].v1 = bottom;
    v[3].u1 = right;
    v[3].v1 = top;
    
    // oldcode
    IDirect3DVertexBuffer9_Unlock(vbuf);
}

static void fixtexcoord_mode1_fixvert(struct fvf_xyztex2 *v)
{
    fRECT src_frect, dst_frect;
    set_frect_ltrb(&src_frect, 0.1, 0.1, 1.0, 1.0);
    if (!RenderTarget_Inst()->m_iMode) {
        set_frect_ltrb(&dst_frect, 0.0, 0.0, game_frect_sqrtex.right, game_frect_sqrtex.bottom);
    } else {
        set_frect_ltrb(&dst_frect, 0.0, 0.0, 1.0, 1.0);
    }
    int i;
    int n = 100;
    for (i = 0; i < n; i++) {
        fPOINT fpoint;
        set_fpoint(&fpoint, v[i].u1, v[i].v1);
        transform_fpoint(&fpoint, &fpoint, &src_frect, &dst_frect, TR_SCALE_LOW, TR_SCALE_LOW, 1.0);
        v[i].u1 = fpoint.x;
        v[i].v1 = fpoint.y;
    }
}

static MAKE_ASMPATCH(fixtexcoord_mode1_part2)
{
    struct fvf_xyztex2 *v = TOPTR(M_DWORD(R_ESP + 0x1C));
    struct UnderWater *this = TOPTR(R_ESI);
    IDirect3DVertexBuffer9 *vbuf = this->m_pVB;
    
    // fix vert tex coord
    fixtexcoord_mode1_fixvert(v);
    
    // oldcode
    IDirect3DVertexBuffer9_Unlock(vbuf);
}


MAKE_PATCHSET(fixunderwater)
{
    // let RenderTarget use m_ScreenPlane instead of m_Texture
    SIMPLE_PATCH(0x004BFD78, "\x6A\x02", "\x6A\x01", 2);
    
    // normalize tex coord regardless of RenderTarget::m_iMode and UnderWater::m_iMode
    SIMPLE_PATCH(0x004BF81F, "\xC7\x44\x24\x24\x00\x00\x40\x3F", "\xC7\x44\x24\x24\x00\x00\x80\x3F", 8); // UnderWater::Create
    SIMPLE_PATCH(0x004C0305, "\xC7\x44\x24\x08\x00\x00\x40\x3F", "\xC7\x44\x24\x08\x00\x00\x80\x3F", 8); // UnderWater::Update

    // fix tex coord for mode 0
    INIT_ASMPATCH(fixtexcoord_mode0_part1, 0x004BFB25, 6, "\x50\x8B\x10\xFF\x52\x30"); // UnderWater::Create
    INIT_ASMPATCH(fixtexcoord_mode0_part2, 0x004C02DA, 9, "\x8B\x76\x08\x56\x8B\x06\xFF\x50\x30"); // UnderWater::Update

    // fix tex coord for mode 1
    INIT_ASMPATCH(fixtexcoord_mode1_part2, 0x004C03EB, 9, "\x8B\x76\x08\x56\x8B\x06\xFF\x50\x30"); // UnderWater::Update

    // debug purpose only
    //SIMPLE_PATCH_NOP(0x4bf7b7, "\x75\x07", 2); // force UnderWater::m_iMode == 1 (use vertex instead of bumpmap)
    //SIMPLE_PATCH(0x4bdbd9, "\xFF\x50\x0C", "\x83\xC4\x0C", 3); // force RenderTarget::m_Texture (use square texture instead of screen plane)
}
