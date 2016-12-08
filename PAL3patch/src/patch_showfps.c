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
static LARGE_INTEGER qwLastTime;

#define JITTER_LIMIT 0.03 // relative factor

#define MAX_JITTER_QUEUE 10
#define MAX_JITTER_SHOWTIME 3.0
struct jitter_info_t {
    LARGE_INTEGER dwTimeStamp;
    double period;
    double fps;
};
static struct jitter_info_t jitter_info[MAX_JITTER_QUEUE];

static void showfps_calcfps()
{
    fcnt++;
    QueryPerformanceCounter(&qwTime);
    cur_time = (qwTime.QuadPart - qwStartTime.QuadPart) / (double) qwTicksPerSec.QuadPart;
    
    // update fps
    if (cur_time - last_time >= 0.5) {
        fps = fcnt / (cur_time - last_time);
        fcnt = 0;
        last_time = cur_time;
    }
    
    // update jitter info
    double cur_period = (qwTime.QuadPart - qwLastTime.QuadPart) / (double) qwTicksPerSec.QuadPart;
    int i;
    for (i = 0; i < MAX_JITTER_QUEUE; i++) {
        if ((qwTime.QuadPart - jitter_info[i].dwTimeStamp.QuadPart) / (double) qwTicksPerSec.QuadPart > MAX_JITTER_SHOWTIME) {
            jitter_info[i].dwTimeStamp.QuadPart = 0;
        }
    }
    if (fabs(cur_period * fps - 1.0) >= JITTER_LIMIT) {
        memmove(&jitter_info[0], &jitter_info[1], sizeof(struct jitter_info_t) * (MAX_JITTER_QUEUE - 1));
        jitter_info[MAX_JITTER_QUEUE - 1] = (struct jitter_info_t) {
            .dwTimeStamp = qwTime,
            .period = cur_period,
            .fps = fps,
        };
    }
    qwLastTime = qwTime;
}

static void showfps_postpresent()
{
    showfps_calcfps();
}
static void showfps_onendscene()
{
    if (!pFPSFont) return;

    char str[MAXLINE];
    char *ptr = str;
    *ptr = '\0';
    
    // generate jitter info
    int i;
    for (i = 0; i < MAX_JITTER_QUEUE; i++) {
        struct jitter_info_t *ji = &jitter_info[i];
        if (ji->dwTimeStamp.QuadPart > 0) {
            snprintf(ptr, str + sizeof(str) - ptr, " %+.3fms (%+.2f%%)\n", (ji->period - 1.0 / ji->fps) * 1000.0, (ji->period * ji->fps - 1.0) * 100.0);
            ptr += strlen(ptr);
        }
    }
    
    wchar_t buf[MAXLINE];
    snwprintf(buf, sizeof(buf) / sizeof(wchar_t), L"PAL3patch %hs\n%hs\nFPS = %.3f\n\n%hs", patch_version, build_info, fps, str);

    
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
    add_postpresent_hook(showfps_postpresent);
    
    memset(jitter_info, 0, sizeof(jitter_info));
}
