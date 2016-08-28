#include <d3d9.h>
#include "common.h"
#include "gbengine.h"


// fix gbGfxManager_D3D::SetRenderType, this is decompiled from PAL3A's GBENGINE.DLL
#define g_pLastRenderTarget (*(IDirect3DSurface9 **) (gboffset + 0x1013BDF4))
static int __fastcall gbGfxManager_D3D_SetRenderTarget(struct gbGfxManager_D3D *this, int dummy, struct gbTexture_D3D *pNewTex)
{
    IDirect3DSurface9 *pNewSurface = NULL;
    if (pNewTex) {
        if (IDirect3DDevice9_GetRenderTarget(this->m_pd3dDevice, 0, &g_pLastRenderTarget) < 0) {
            g_pLastRenderTarget = NULL;
            return 0;
        }
        IDirect3DSurface9_Release(g_pLastRenderTarget);
        if (!pNewTex->pTex) return 0;
        IDirect3DTexture9_GetSurfaceLevel((IDirect3DTexture9 *) pNewTex->pTex, 0, &pNewSurface);
        if (!pNewSurface) return 0;
        return IDirect3DDevice9_SetRenderTarget(this->m_pd3dDevice, 0, pNewSurface) >= 0;
    } else {
        if (g_pLastRenderTarget) {
            if (IDirect3DDevice9_GetRenderTarget(this->m_pd3dDevice, 0, &pNewSurface) >= 0) {
                IDirect3DSurface9_Release(pNewSurface); // release twice, since GetRenderTarget() will increase ref count
                IDirect3DSurface9_Release(pNewSurface);
            }
            IDirect3DDevice9_SetRenderTarget(this->m_pd3dDevice, 0, g_pLastRenderTarget);
        }
        return 1;
    }
}

// these are some function decompiled from PAL3A
// no need to emulate __thiscall

static void gbTexture_D3D_ReleaseD3D(struct gbTexture_D3D *this)
{
    if (this->pTex) IDirect3DBaseTexture9_Release(this->pTex);
    if (this->pDS) IDirect3DSurface9_Release(this->pDS);
    this->pTex = NULL;
    this->pDS = NULL;
    this->m_ImgFormat = 0;
}
static void RenderTarget_Create(struct RenderTarget *this)
{
    void __fastcall (*RenderTarget_Create_Real)(struct RenderTarget *this); // only 1 argument, no need to add dummy
    RenderTarget_Create_Real = TOPTR(0x004BDB60);
    RenderTarget_Create_Real(this);
}
static void RenderTarget_OnDeviceLost(struct RenderTarget *this)
{
    gbTexture_D3D_ReleaseD3D(&this->m_Texture);
    gbTexture_D3D_ReleaseD3D(&this->m_ScreenPlane);
}
static BYTE RenderTarget_OnResetDevice(struct RenderTarget *this)
{
    int m_iMode_save = this->m_iMode;
    int m_nState_save = this->m_nState;
    RenderTarget_Create(this);
    this->m_iMode = m_iMode_save;
    this->m_nState =  m_nState_save;
    return 1;
}

static MAKE_ASMPATCH(fixreset_RenderTarget_End_patch)
{
    IDirect3DBaseTexture9_Release((IDirect3DBaseTexture9 *)(M_DWORD(R_ESP + 0x0C)));
    IDirect3DBaseTexture9_Release((IDirect3DBaseTexture9 *)(M_DWORD(R_ESP + 0x10)));
}





#define pRenderTarget ((struct RenderTarget *) 0x00DE7AA0)
static void OnDeviceLost_hook()
{
    RenderTarget_OnDeviceLost(pRenderTarget);
}
static void OnResetDevice_hook()
{
    RenderTarget_OnResetDevice(pRenderTarget);
}





static MAKE_ASMPATCH(fixreset_devicelost_hook)
{
    OnDeviceLost_hook();
    R_ECX = M_DWORD(R_ESI + 0xC4); // oldcode
}
static MAKE_ASMPATCH(fixreset_resetdevice_hook)
{
    OnResetDevice_hook();
    R_ECX = M_DWORD(R_ESI + 0xC4); // oldcode
}






static MAKE_ASMPATCH(fixreset_gbGfxManager_D3D_BeginScene_patch)
{
    gbGfxManager_D3D_EnsureCooperativeLevel(TOPTR(R_ECX), 0);
    
    PUSH_DWORD(R_EBP); // oldcode
    PUSH_DWORD(R_ESI);
    PUSH_DWORD(R_EDI);
    R_ESI = R_ECX;
}

static MAKE_ASMPATCH(fixreset_UnderWater_1)
{
    PUSH_DWORD(D3DPOOL_MANAGED);
    PUSH_DWORD(D3DFMT_INDEX16);
    PUSH_DWORD(D3DUSAGE_WRITEONLY);
}

MAKE_PATCHSET(fixreset)
{
    // fix gbGfxManager_D3D::SetRenderTarget
    make_jmp(gboffset + 0x1001B360, gbGfxManager_D3D_SetRenderTarget);
    
    // install DeviceLost and ResetDevice hooks
    INIT_ASMPATCH(fixreset_devicelost_hook, gboffset + 0x1001AC56, 6, "\x8B\x8E\xC4\x00\x00\x00");
    INIT_ASMPATCH(fixreset_resetdevice_hook, gboffset + 0x1001AD5E, 6, "\x8B\x8E\xC4\x00\x00\x00");
    
    // fix RenderTarget
    SIMPLE_PATCH(0x004BDE8F, "\x8B\x54\x24\x20\x52\x50\xFF", "\xFF\x74\xE4\x20\x50\xEB\x12", 7);
    SIMPLE_PATCH(0x004BDEA8, "\x90\x90\x90\x90\x90\x90\x90\x90", "\xFF\x91\x88\x00\x00\x00\xEB\xE6", 8);
    INIT_ASMPATCH(fixreset_RenderTarget_End_patch, 0x004BDE96, 5, "\x91\x88\x00\x00\x00");
    
    // fix UnderWater
    INIT_ASMPATCH(fixreset_UnderWater_1, 0x004BFB6B, 8, "\x53\x6A\x65\x68\x00\x02\x00\x00");
    SIMPLE_PATCH(0x004BFC58, "\x68\x00\x20\x00\x00", "\x68\x00\x00\x00\x00", 5);
    
    // patch gbGfxManager_D3D::BeginScene
    INIT_ASMPATCH(fixreset_gbGfxManager_D3D_BeginScene_patch, gboffset + 0x10018CD3, 5, "\x55\x56\x57\x8B\xF1");

    // FIXME: may be more patch needed!
}
