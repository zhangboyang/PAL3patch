#include "common.h"

static int screenshot_flag = 0;

#define SCREENSHOT_MSG_TIME 5000
static wchar_t screenshot_msg[MAXLINE];
static DWORD screenshot_msg_time;
static int screenshot_msg_enable = 0;

static void screenshot_hook()
{
    if (screenshot_flag) {

        // create directory
        CreateDirectory("snap", NULL);
        
        // prepare filename
        char buf[MAXLINE];
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);
        snprintf(buf, sizeof(buf), "snap\\%04hu%02hu%02hu_%02hu%02hu%02hu_%03hu.bmp", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
    
        // save image
        IDirect3DSurface9 *suf = NULL;
        if (SUCCEEDED(IDirect3DDevice9_GetBackBuffer(GB_GfxMgr->m_pd3dDevice, 0, 0, D3DBACKBUFFER_TYPE_MONO, &suf))) {
            if (SUCCEEDED(D3DXSaveSurfaceToFileA(buf, D3DXIFF_BMP, suf, NULL, NULL))) {
                snwprintf(screenshot_msg, sizeof(screenshot_msg) / sizeof(wchar_t), wstr_screenshot_msg, buf);
                screenshot_msg_time = timeGetTime();
                screenshot_msg_enable = 1;
            } else {
                warning("screenshot failed.");
            }
            IDirect3DSurface9_Release(suf);
        }
        
        // reset flag
        screenshot_flag = 0;
    }
    
    if (screenshot_msg_enable) {
        print_wstring_begin();
        print_wstring(FONTID_U12_SCALED, screenshot_msg, 12 * game_scalefactor, 12 * game_scalefactor, 0xFFFFFF00);
        print_wstring_end();
        if (timeGetTime() - screenshot_msg_time >= SCREENSHOT_MSG_TIME) {
            screenshot_msg_enable = 0;
        }
    }
}


static int screenshot_enabled = 0;
// this function may be called by outside functions
int try_screenshot()
{
    if (!screenshot_enabled) return 0;
    screenshot_flag = 1;
    return 1;
}

static void screenshot_wndproc_hook(void *arg)
{
    struct wndproc_hook_data *data = arg;
    
    if (data->Msg == WM_KEYUP && data->wParam == VK_F8) {
        if (try_screenshot()) {
            data->retvalue = 0;
            data->processed = 1;
        }
    }
}

static void screenshot_grpkbdstate_hook()
{
    g_input.m_keyRaw[DIK_F8] = 0;
}

MAKE_PATCHSET(screenshot)
{
    screenshot_enabled = 1;
    add_preendscene_hook(screenshot_hook);
    add_postwndproc_hook(screenshot_wndproc_hook);
    add_grpkbdstate_hook(screenshot_grpkbdstate_hook);
}
