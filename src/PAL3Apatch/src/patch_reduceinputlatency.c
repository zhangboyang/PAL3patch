#include "common.h"



// method1: reduce latency by doing event query after presenting each frame
static void method1_hook()
{
    IDirect3DQuery9 *pQuery;
    if (IDirect3DDevice9_CreateQuery(GB_GfxMgr->m_pd3dDevice, D3DQUERYTYPE_EVENT, &pQuery) == D3D_OK) {
        if (pQuery) {
            IDirect3DQuery9_Issue(pQuery, D3DISSUE_END);
            while (IDirect3DQuery9_GetData(pQuery, NULL, 0, D3DGETDATA_FLUSH) == S_FALSE); // busy wait
            IDirect3DQuery9_Release(pQuery);
        }
    }
}
static void method1_init()
{
    add_postpresent_hook(method1_hook);
}







// method2: lock backbuffer before endscene, force flush command buffer

static IDirect3DSurface9 *method2_pRenderTarget = NULL;
static void method2_release_rendertarget()
{
    if (method2_pRenderTarget) {
        IDirect3DSurface9_Release(method2_pRenderTarget);
        method2_pRenderTarget = NULL;
    }
}
static void method2_create_rendertarget()
{
    assert(method2_pRenderTarget == NULL);
    
    // get backbuffer format
    D3DSURFACE_DESC desc;
    IDirect3DSurface9 *pBackBuffer = NULL;
    if (FAILED(IDirect3DDevice9_GetBackBuffer(GB_GfxMgr->m_pd3dDevice, 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer))) {
        pBackBuffer = NULL;
        goto done;
    }
    if (FAILED(IDirect3DSurface9_GetDesc(pBackBuffer, &desc))) {
        goto done;
    }
    
    if (FAILED(IDirect3DDevice9_CreateRenderTarget(GB_GfxMgr->m_pd3dDevice, desc.Width, desc.Height, desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &method2_pRenderTarget, NULL))) {
        method2_pRenderTarget = NULL;
        goto done;
    }
    
done:
    if (pBackBuffer) IDirect3DSurface9_Release(pBackBuffer);
}
static void method2_onlostdevice()
{
    method2_release_rendertarget();
}
static void method2_onresetdevice()
{
    method2_create_rendertarget();
}
static void method2_postd3dcreate_hook()
{
    method2_create_rendertarget();
}
static void method2_preendscene_hook()
{
    if (!method2_pRenderTarget) return;
    
    IDirect3DSurface9 *pBackBuffer = NULL;
    // get back buffer
    if (FAILED(IDirect3DDevice9_GetBackBuffer(GB_GfxMgr->m_pd3dDevice, 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer))) {
        warning("can't get back buffer.");
        pBackBuffer = NULL;
        goto done;
    }
    // copy backbuffer to render target
    if (FAILED(IDirect3DDevice9_StretchRect(GB_GfxMgr->m_pd3dDevice, pBackBuffer, NULL, method2_pRenderTarget, NULL, D3DTEXF_POINT))) {
        warning("can't stretch rect.");
        goto done;
    }
    // lock the render target
    D3DLOCKED_RECT lrect;
    if (FAILED(IDirect3DSurface9_LockRect(method2_pRenderTarget, &lrect, NULL, 0))) {
        warning("can't lock rect.");
        goto done;
    }
    // FIXME: should we touch the memory at lrect.pBits
    // unlock the render target
    IDirect3DSurface9_UnlockRect(method2_pRenderTarget);
    
done:
    if (pBackBuffer) IDirect3DSurface9_Release(pBackBuffer);
}
static void method2_init()
{
    add_postd3dcreate_hook(method2_postd3dcreate_hook);
    add_preendscene_hook(method2_preendscene_hook);
    add_onlostdevice_hook(method2_onlostdevice);
    add_onresetdevice_hook(method2_onresetdevice);
}



MAKE_PATCHSET(reduceinputlatency)
{
    switch (flag) {
        case 1: method1_init(); break;
        case 2: method2_init(); break;
        default: fail("invalid reduce input latency configuration %d.", flag);
    }
}
