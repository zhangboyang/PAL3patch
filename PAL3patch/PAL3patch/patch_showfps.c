#include "common.h"

static ID3DXFont *pFPSFont = NULL;
static ID3DXSprite *pFPSSprite = NULL;
static IDirect3DStateBlock9 *pFPSStateBlock = NULL;
static int fcnt = 0; // frame count
static double fps = 0;
static double cur_time = 0;
static double last_time = 0;
static LARGE_INTEGER qwTicksPerSec;
static LARGE_INTEGER qwTime;
static LARGE_INTEGER qwStartTime;

static void showfps_calcfps()
{
    fcnt++;
    QueryPerformanceCounter(&qwTime);
    cur_time = (qwTime.QuadPart - qwStartTime.QuadPart) / (double) qwTicksPerSec.QuadPart;
    if (cur_time - last_time >= 0.5) {
        fps = fcnt / (cur_time - last_time);
        fcnt = 0;
        last_time = cur_time;
    }
}

static void showfps_onendscene()
{
    if (!pFPSFont) return;
    
    showfps_calcfps();
    
    wchar_t buf[MAXLINE];
    snwprintf(buf, sizeof(buf) / sizeof(wchar_t), L"PAL3patch %hs\n%hs\nFPS = %.1f\n", PAL3PATCH_VERSION, build_info, fps);
    
    IDirect3DStateBlock9_Capture(pFPSStateBlock);
    
    // make text in front of other pixels
    IDirect3DDevice9_SetRenderState(g_GfxMgr->m_pd3dDevice, D3DRS_ZFUNC, D3DCMP_ALWAYS);
    
    // manually turn on alphablend, for drawing charactor backgrounds
    IDirect3DDevice9_SetRenderState(g_GfxMgr->m_pd3dDevice, D3DRS_ALPHABLENDENABLE, TRUE);
    IDirect3DDevice9_SetRenderState(g_GfxMgr->m_pd3dDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    IDirect3DDevice9_SetRenderState(g_GfxMgr->m_pd3dDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO);
    IDirect3DDevice9_SetTextureStageState(g_GfxMgr->m_pd3dDevice, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(g_GfxMgr->m_pd3dDevice, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    
    ID3DXSprite_Begin(pFPSSprite, 0);
    RECT rc;
    set_rect(&rc, 10, 10, 0, 0);
    ID3DXFont_DrawTextW(pFPSFont, pFPSSprite, buf, -1, &rc, DT_NOCLIP, 0xFFFFFFFF);
    ID3DXSprite_End(pFPSSprite);
    
    IDirect3DStateBlock9_Apply(pFPSStateBlock);
}
static void showfps_onlostdevice()
{
    if (!pFPSFont) return;
    ID3DXFont_OnLostDevice(pFPSFont);
    ID3DXSprite_OnLostDevice(pFPSSprite);
    IDirect3DStateBlock9_Release(pFPSStateBlock);
}
static void showfps_onresetdevice()
{
    if (!pFPSFont) return;
    ID3DXFont_OnResetDevice(pFPSFont);
    ID3DXSprite_OnResetDevice(pFPSSprite);
    
    if (FAILED(IDirect3DDevice9_CreateStateBlock(g_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &pFPSStateBlock))) {
        fail("can't create state block for showing FPS.");
    }
}
static void showfps_initfont()
{
    if (FAILED(D3DXCreateFontW(g_GfxMgr->m_pd3dDevice, 12, 0, 0, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Fixedsys", &pFPSFont))) {
        warning("can't create font for showing FPS.");
        pFPSFont = NULL;
    }
    
    if (FAILED(D3DXCreateSprite(g_GfxMgr->m_pd3dDevice, &pFPSSprite))) {
        fail("can't create sprite for showing FPS.");
    }
    
    if (FAILED(IDirect3DDevice9_CreateStateBlock(g_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &pFPSStateBlock))) {
        fail("can't create state block for showing FPS.");
    }
}
MAKE_PATCHSET(showfps)
{
    if (!QueryPerformanceFrequency(&qwTicksPerSec)) {
        // we can't use QueryPerformanceCounter
        warning("can't show fps.");
        return;
    }
    QueryPerformanceCounter(&qwStartTime);
    
    add_postd3dcreate_hook(showfps_initfont);
    add_onlostdevice_hook(showfps_onlostdevice);
    add_onresetdevice_hook(showfps_onresetdevice);
    add_preendscene_hook(showfps_onendscene);
}
