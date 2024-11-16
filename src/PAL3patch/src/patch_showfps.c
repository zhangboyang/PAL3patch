#include "common.h"

static int showver_flag;
static char vstr[MAXLINE];

static int showdev_flag;
static wchar_t dstr[MAXLINE];

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

#define MAX_JITTER_QUEUE 20
#define MAX_JITTER_SHOWTIME 5.0
struct jitter_info_t {
    LARGE_INTEGER dwTimeStamp;
    double period;
    double standard_fps;
};
static double jitter_limit;
static double standard_fps1, standard_fps2;
static struct jitter_info_t jitter_info[MAX_JITTER_QUEUE];

static void showfps_calcfps()
{
    fcnt++;
    QueryPerformanceCounter(&qwTime);
    cur_time = (qwTime.QuadPart - qwStartTime.QuadPart) / (double) qwTicksPerSec.QuadPart;
    
    if (jitter_limit >= 0 && qwLastTime.QuadPart != 0) {
        // update jitter info
        double cur_period = (qwTime.QuadPart - qwLastTime.QuadPart) / (double) qwTicksPerSec.QuadPart;
        double standard_fps = 
            fabs(standard_fps1 - fps) <= fabs(standard_fps2 - fps) ||
            fabs(1.0 / cur_period - standard_fps1) <= fabs(1.0 / cur_period - standard_fps2)
                ? standard_fps1 : standard_fps2;
        
        int i;
        for (i = 0; i < MAX_JITTER_QUEUE; i++) {
            if ((qwTime.QuadPart - jitter_info[i].dwTimeStamp.QuadPart) / (double) qwTicksPerSec.QuadPart > MAX_JITTER_SHOWTIME) {
                jitter_info[i].dwTimeStamp.QuadPart = 0;
            }
        }
        if (fabs(cur_period * standard_fps - 1.0) >= jitter_limit) {
            memmove(&jitter_info[0], &jitter_info[1], sizeof(struct jitter_info_t) * (MAX_JITTER_QUEUE - 1));
            jitter_info[MAX_JITTER_QUEUE - 1] = (struct jitter_info_t) {
                .dwTimeStamp = qwTime,
                .period = cur_period,
                .standard_fps = standard_fps,
            };
        }
    }

    // update fps
    if (cur_time - last_time >= 0.5) {
        fps = fcnt / (cur_time - last_time);
        fcnt = 0;
        last_time = cur_time;
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

    char jstr[MAXLINE];
    char *ptr = jstr;
    *ptr = '\0';
    
    // generate jitter info
    if (jitter_limit >= 0) {
        int i;
        for (i = 0; i < MAX_JITTER_QUEUE; i++) {
            struct jitter_info_t *ji = &jitter_info[i];
            if (ji->dwTimeStamp.QuadPart > 0) {
                snprintf(ptr, jstr + sizeof(jstr) - ptr, 
                    " %+.3fms (%+.2f%%, @%.1f)\n",
                    (ji->period - 1.0 / ji->standard_fps) * 1000.0,
                    (ji->period * ji->standard_fps - 1.0) * 100.0,
                    ji->standard_fps
                );
                ptr += strlen(ptr);
            }
        }
    }
    
    wchar_t buf[MAXLINE];
    snwprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%hs%sFPS = %.3f\n\n%hs", vstr, dstr, fps, jstr);

    
    IDirect3DStateBlock9_Capture(pFPSStateBlock);
    
    // make text in front of other pixels
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ZFUNC, D3DCMP_ALWAYS);
    
    // manually turn on alphablend, for drawing charactor backgrounds
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ALPHABLENDENABLE, TRUE);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ALPHATESTENABLE, FALSE);
    IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    
    // setup matrices manually
    set_d3dxfont_matrices(GB_GfxMgr->m_pd3dDevice);
    
    // draw chars
    myID3DXSprite_Begin(pFPSSprite, D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(GB_GfxMgr->m_pd3dDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    
    RECT rc;
    set_rect(&rc, 10, 10, 0, 0);
    myID3DXFont_DrawTextW(pFPSFont, pFPSSprite, buf, -1, &rc, DT_NOCLIP, 0xFFFFFFFF);
    myID3DXSprite_End(pFPSSprite);
    
    IDirect3DStateBlock9_Apply(pFPSStateBlock);
}
static void showfps_onlostdevice()
{
    if (!pFPSFont) return;
    myID3DXFont_OnLostDevice(pFPSFont);
    myID3DXSprite_OnLostDevice(pFPSSprite);
    IDirect3DStateBlock9_Release(pFPSStateBlock);
}
static void showfps_onresetdevice()
{
    if (!pFPSFont) return;
    myID3DXFont_OnResetDevice(pFPSFont);
    myID3DXSprite_OnResetDevice(pFPSSprite);
    
    if (FAILED(IDirect3DDevice9_CreateStateBlock(GB_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &pFPSStateBlock))) {
        fail("can't create state block for showing FPS.");
    }
}
static void showfps_initfont()
{
    if (FAILED(myD3DXCreateFontW(GB_GfxMgr->m_pd3dDevice, 12, 0, 0, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Fixedsys", &pFPSFont))) {
        warning("can't create font for showing FPS.");
        pFPSFont = NULL;
    }
    
    if (FAILED(myD3DXCreateSprite(GB_GfxMgr->m_pd3dDevice, &pFPSSprite))) {
        fail("can't create sprite for showing FPS.");
    }
    
    if (FAILED(IDirect3DDevice9_CreateStateBlock(GB_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &pFPSStateBlock))) {
        fail("can't create state block for showing FPS.");
    }
    
    if (showver_flag) {
        snprintf(vstr, sizeof(vstr), "PAL3patch %s\n%s\n", patch_version, build_info);
    }
    
    if (showdev_flag) {
        D3DDEVICE_CREATION_PARAMETERS Parameters;
        if (IDirect3DDevice9_GetCreationParameters(GB_GfxMgr->m_pd3dDevice, &Parameters) == D3D_OK) {
            IDirect3D9 *pD3D9;
            if (IDirect3DDevice9_GetDirect3D(GB_GfxMgr->m_pd3dDevice, &pD3D9) == D3D_OK) {
                D3DADAPTER_IDENTIFIER9 Identifier;
                if (IDirect3D9_GetAdapterIdentifier(pD3D9, Parameters.AdapterOrdinal, 0, &Identifier) == D3D_OK) {
                    wchar_t *s = cs2wcs_alloc(Identifier.Description, CP_ACP);
                    if (s) {
                        snwprintf(dstr, sizeof(dstr) / sizeof(wchar_t), L"%s\n\n", s);
                        free(s);
                    }
                }
                IDirect3D9_Release(pD3D9);
            }
        }
    }
}
MAKE_PATCHSET(showfps)
{
    showver_flag = get_int_from_configfile("showfps_showversion");
    showdev_flag = get_int_from_configfile("showfps_showdevice");

    const char *jitter_cfgstr = get_string_from_configfile("showfps_showjitter");
    if (sscanf(jitter_cfgstr, "%lf,%lf,%lf", &jitter_limit, &standard_fps1, &standard_fps2) != 3) {
        fail("invalid showfps showjitter config string '%s'.", jitter_cfgstr);
    }
    
    
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
