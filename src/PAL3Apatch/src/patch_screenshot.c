#include "common.h"

static int screenshot_flag = 0;

static void screenshot_hook()
{
    if (!screenshot_flag) return;

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
        if (FAILED(D3DXSaveSurfaceToFileA(buf, D3DXIFF_BMP, suf, NULL, NULL))) {
            warning("screenshot failed.");
        }
        IDirect3DSurface9_Release(suf);
    }
    
    // reset flag
    screenshot_flag = 0;
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

static void PAL3_PrintScreen(int width, int height)
{
    //try_screenshot();
    // do NOTHING here since we processed keyboard events in WndProc
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
    make_jmp(0x00408699, PAL3_PrintScreen);
    
    add_grpkbdstate_hook(screenshot_grpkbdstate_hook);
}
