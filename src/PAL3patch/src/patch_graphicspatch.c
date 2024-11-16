#include "common.h"

// game resolution
int game_width, game_height;
fRECT game_frect, game_frect_43, game_frect_original;
fRECT game_frect_custom[MAX_CUSTOM_GAME_FRECT];
fRECT game_frect_sqrtex; // normalized ratio rect for square texture
double game_scalefactor;


// fps limiter

static LARGE_INTEGER fpslimit_qwTicksPerSec;
static LARGE_INTEGER fpslimit_qwLast;
static int fpslimit_enabled;
static double fpslimit_target_period;
static double default_fps = -1;
static double standard_fps = 60;

static void set_fpslimit(double target_fps) // if target_fps is zero or negative, disable fpslimit
{
    if (target_fps > 0) {
        fpslimit_target_period = 1.0 / target_fps;
        fpslimit_enabled = 1;
    } else {
        fpslimit_enabled = 0;
    }
}
static void update_fpslimit()
{
    double fps = default_fps;
    switch (PAL3_s_gamestate) {
        case GAME_SUBGAME_ENCAMPMENT:
        case GAME_SUBGAME_SKEE:
        case GAME_SUBGAME_ROWING:
        case GAME_UI:
            fps = standard_fps; break;
    }
    set_fpslimit(fps);
}
static void fpslimit_hook()
{
    update_fpslimit();
    if (fpslimit_qwTicksPerSec.QuadPart > 0) {
        LARGE_INTEGER qwTime;
        while (1) {
            QueryPerformanceCounter(&qwTime);
            if (!fpslimit_enabled) break;
            double diff = (qwTime.QuadPart - fpslimit_qwLast.QuadPart) / (double) fpslimit_qwTicksPerSec.QuadPart;
            if (diff >= fpslimit_target_period) break;
            int sleepms = floor((fpslimit_target_period - diff) * 1000.0);
            if (sleepms > 2) Sleep(sleepms - 2);
        }
        fpslimit_qwLast.QuadPart = qwTime.QuadPart;
    }
}
static void fpslimit_init()
{
    if (sscanf(get_string_from_configfile("game_fpslimit"), "%lf,%lf", &default_fps, &standard_fps) != 2) {
        fail("invalid game_fpslimit config string.");
    }

    if (!QueryPerformanceFrequency(&fpslimit_qwTicksPerSec)) {
        warning("can't query performance frequency.");
        fpslimit_qwTicksPerSec.QuadPart = 0;
    }
    fpslimit_qwLast.QuadPart = 0;
    
    add_postpresent_hook(fpslimit_hook);
}



// part of the CArrayList class, from DirectX SDK C++ Sample
enum ArrayListType { AL_VALUE, AL_REFERENCE };
struct CArrayList {
    enum ArrayListType m_ArrayListType;
    void *m_pData;
    UINT m_BytesPerEntry;
    UINT m_NumEntries;
    UINT m_NumEntriesAllocated;
};
static void *CArrayList_GetPtr(struct CArrayList *this, UINT Entry)
{
    if (this->m_ArrayListType == AL_VALUE)
        return (BYTE*) this->m_pData + (Entry * this->m_BytesPerEntry);
    else
        return *(((void**) this->m_pData) + Entry);
}
static int *CArrayList_FindInt(struct CArrayList *this, int target, int *subscript)
{
    int i;
    for (i = 0; i < (int) this->m_NumEntries; i++) {
        int *cur = CArrayList_GetPtr(this, i);
        if (*cur == target) {
            if (subscript) *subscript = i;
            return cur;
        }
    }
    if (subscript) *subscript = -1;
    return NULL;
}






// the Z-buffer patch
static int zbuf_flag;
static int zbuf_tmpint;
static MAKE_ASMPATCH(zbuf)
{
    struct CArrayList *this = TOPTR(R_ECX);
    if (zbuf_flag == INT_MAX) {
        int *result = NULL;
        if (!result) result = CArrayList_FindInt(this, D3DFMT_D24S8, NULL);
        if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X8, NULL);
        if (!result) result = CArrayList_FindInt(this, D3DFMT_D16, NULL);
        if (!result) result = CArrayList_GetPtr(this, 0);
        R_EAX = TOUINT(result);
        return;
    } else if (zbuf_flag < 0) {
        zbuf_tmpint = -zbuf_flag;
        R_EAX = TOUINT(&zbuf_tmpint);
        return;
    } else {
        int *result = NULL;
        switch (zbuf_flag) {
            case 16:
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D16, NULL);
                break;
            case 24:
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24S8, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X8, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X4S4, NULL);
                break;
            case 32:
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D32, NULL);
                break;
        }
        if (!result) {
            result = CArrayList_GetPtr(this, 0);
            warning("z-buffer format not supported, fallback to default configuration.");
        }
        R_EAX = TOUINT(result);
        return;
    }
}
static void patch_depth_buffer_config(const char *cfgstr)
{
    zbuf_flag = (stricmp(cfgstr, "auto") == 0) ? INT_MAX : str2int(cfgstr);
    if (zbuf_flag) {
        INIT_ASMPATCH(zbuf, gboffset + 0x10019E13, 7, "\x6A\x00\xE8\x96\xD0\xFF\xFF");
        INIT_ASMPATCH(zbuf, gboffset + 0x1001A12E, 7, "\x6A\x00\xE8\x7B\xCD\xFF\xFF");
    }
}






// multisample patch
static int ms_tflag, ms_qflag;
static void set_multisample_config(struct CArrayList *tl, struct CArrayList *ql, int *tret, int *qret)
{
    if (ms_tflag < 0 || ms_qflag < 0) {
        *tret = -ms_tflag;
        *qret = -ms_qflag;
        return;
    } 
    const D3DMULTISAMPLE_TYPE msTypeArray[] = { 
        D3DMULTISAMPLE_NONE,
        D3DMULTISAMPLE_NONMASKABLE,
        D3DMULTISAMPLE_2_SAMPLES,
        D3DMULTISAMPLE_3_SAMPLES,
        D3DMULTISAMPLE_4_SAMPLES,
        D3DMULTISAMPLE_5_SAMPLES,
        D3DMULTISAMPLE_6_SAMPLES,
        D3DMULTISAMPLE_7_SAMPLES,
        D3DMULTISAMPLE_8_SAMPLES,
        D3DMULTISAMPLE_9_SAMPLES,
        D3DMULTISAMPLE_10_SAMPLES,
        D3DMULTISAMPLE_11_SAMPLES,
        D3DMULTISAMPLE_12_SAMPLES,
        D3DMULTISAMPLE_13_SAMPLES,
        D3DMULTISAMPLE_14_SAMPLES,
        D3DMULTISAMPLE_15_SAMPLES,
        D3DMULTISAMPLE_16_SAMPLES,
    };
    const UINT msTypeArrayCount = sizeof(msTypeArray) / sizeof(msTypeArray[0]);
    int id;
    if (ms_tflag == INT_MAX) {
        int *pt = NULL;
        if (!pt) pt = CArrayList_FindInt(tl, D3DMULTISAMPLE_8_SAMPLES, &id);
        if (!pt) pt = CArrayList_FindInt(tl, D3DMULTISAMPLE_4_SAMPLES, &id);
        if (!pt) pt = CArrayList_FindInt(tl, D3DMULTISAMPLE_NONE, &id);
        if (!pt) {
            id = 0;
            pt = CArrayList_GetPtr(tl, id);
        }
        *tret = *pt;
    } else if (ms_tflag >= (int) msTypeArrayCount) {
        fail("invalid multisample type configuration.");
    } else {
        int tprefer = msTypeArray[ms_tflag];
        int *pt = CArrayList_FindInt(tl, tprefer, &id);
        if (!pt) {
            warning("multisample type not supported, fallback to default configuration.");
            *tret = *qret = 0;
            return;
        }
        *tret = *pt;
    }
    int q = *(int *) CArrayList_GetPtr(ql, id);
    if (ms_qflag == INT_MAX) {
        q = 0;
    } else if (ms_qflag >= q) {
        warning("multisample quality %d (max %d) not supported, fallback to default configuration.", ms_qflag, q);
        q = 0;
    } else {
        q = ms_qflag;
    }
    *qret = q;
}
static MAKE_ASMPATCH(multisample_windowed)
{
    set_multisample_config(TOPTR(M_DWORD(R_EBP + 0x18)), TOPTR(M_DWORD(R_EBP + 0x1C)), TOPTR(R_EDI + 0x694), TOPTR(R_EDI + 0x698));
}
static MAKE_ASMPATCH(multisample_fullscreen)
{
    set_multisample_config(TOPTR(M_DWORD(R_ECX + 0x18)), TOPTR(M_DWORD(R_ECX + 0x1C)), TOPTR(R_ESI + 0x6CC), TOPTR(R_ESI + 0x6D0));
}
static void patch_multisample_config(const char *cfgstr)
{
    char buf[MAXLINE];
    strncpy(buf, cfgstr, sizeof(buf)); buf[sizeof(buf) - 1] = 0;
    char *ptr = strchr(buf, ',');
    if (!ptr) fail("can't parse multiplesample config string.");
    *ptr++ = 0;
    ms_tflag = (stricmp(buf, "auto") == 0) ? INT_MAX : str2int(buf);
    ms_qflag = (stricmp(ptr, "auto") == 0) ? INT_MAX : str2int(ptr);
    if (ms_tflag || ms_qflag) {
        INIT_ASMPATCH(multisample_windowed, gboffset + 0x10019E2E , 16, "\xC7\x87\x98\x06\x00\x00\x00\x00\x00\x00\x89\x87\x94\x06\x00\x00");
        INIT_ASMPATCH(multisample_fullscreen, gboffset + 0x1001A15B, 12, "\x89\x86\xCC\x06\x00\x00\x89\xBE\xD0\x06\x00\x00");
    }
}








// resolution patch
static MAKE_THISCALL(BOOL, Readn, void *this, char *appname, char *keyname, int *ret, int defvalue)
{
    if (strcmp(keyname, "width") == 0) { *ret = game_width; return TRUE; }
    if (strcmp(keyname, "height") == 0) { *ret = game_height; return TRUE; }
    fail("invalid call to ConfigFile::Readn");
}
static void patch_resolution_config(const char *cfgstr)
{
    if (stricmp(cfgstr, "current") == 0) {
        game_width = GetSystemMetrics(SM_CXSCREEN);
        game_height = GetSystemMetrics(SM_CYSCREEN);
    } else if (sscanf(cfgstr, "%d %*[xX]%d", &game_width, &game_height) != 2) {
        warning("can't parse resolution string, fallback to default configuration.");
        game_width = GAME_WIDTH_ORG;
        game_height = GAME_HEIGHT_ORG;
    }
    
    if ((long long) game_width * game_height > GAME_WIDTH_SOFTLIMIT * GAME_HEIGHT_SOFTLIMIT) {
        if (MessageBoxW_format(NULL, wstr_resolutiontoolarge_text, wstr_resolutiontoolarge_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND, game_width, game_height, GAME_WIDTH_SOFTLIMIT, GAME_HEIGHT_SOFTLIMIT, game_width, game_height) != IDYES) {
            die(0);
        }
    }
    
    /* calc basic game rect */
    set_frect_ltwh(&game_frect_original, 0, 0, GAME_WIDTH_ORG, GAME_HEIGHT_ORG);
    set_frect_ltwh(&game_frect, 0, 0, game_width, game_height);
    get_ratio_frect(&game_frect_43, &game_frect, 4.0 / 3.0, TR_CENTER, TR_CENTER);
    game_scalefactor = get_frect_min_scalefactor(&game_frect_43, &game_frect_original);

    /* calc ratio rect for square texture */
    set_frect_ltrb(&game_frect_sqrtex, 0.0, 0.0, 1.0, 1.0);
    get_ratio_frect(&game_frect_sqrtex, &game_frect_sqrtex, get_frect_aspect_ratio(&game_frect), TR_LOW, TR_LOW);
        
    /* calc custom game rect */
    int i;
    for (i = 0; i < MAX_CUSTOM_GAME_FRECT; i++) {
        char cfgname[MAXLINE];
        snprintf(cfgname, sizeof(cfgname), "customrect%d", i);
        const char *cfgline = get_string_from_configfile(cfgname);
        double sizefactor, rwidth, rheight;
        if (sscanf(cfgline, "%lf,%lf:%lf", &sizefactor, &rwidth, &rheight) != 3) {
            fail("invalid config line '%s' for custom rect %d.", cfgline, i);
        }
        transform_frect(&game_frect_custom[i], &game_frect, &game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_MID, sizefactor);
        get_ratio_frect(&game_frect_custom[i], &game_frect_custom[i], rwidth / rheight, TR_CENTER, TR_CENTER);
    }

    INIT_WRAPPER_CALL(Readn, {
        0x00406436,
        0x00406453,
    });
}



// refreshrate patch
static int prefered_refreshrate = 60;
static MAKE_ASMPATCH(recalc_fullscreen_refreshrate)
{
    struct gbGfxManager_D3D *this = TOPTR(R_ECX);
    
    struct D3DAdapterInfo *pBestAdapterInfo = this->m_d3dSettings.pFullscreen_AdapterInfo;
    D3DDISPLAYMODE *pBestDisplayMode = &this->m_d3dSettings.Fullscreen_DisplayMode;

    if (prefered_refreshrate > 0) {   
        unsigned idm;
        for (idm = 0; idm < pBestAdapterInfo->pDisplayModeList->m_NumEntries; idm++) {
            D3DDISPLAYMODE *pdm = CArrayList_GetPtr(pBestAdapterInfo->pDisplayModeList, idm);
            if (pBestDisplayMode->Width == pdm->Width && pBestDisplayMode->Height == pdm->Height && pBestDisplayMode->Format == pdm->Format) {
                if (iabs(pdm->RefreshRate - prefered_refreshrate) < iabs(pBestDisplayMode->RefreshRate - prefered_refreshrate)) {
                    pBestDisplayMode->RefreshRate = pdm->RefreshRate;
                }
            }
        }
    }
    
    // oldcode
    R_EAX = pBestDisplayMode->RefreshRate;
}
static void patch_refreshrate_config(const char *cfgstr)
{
    prefered_refreshrate = str2int(cfgstr);
    INIT_ASMPATCH(recalc_fullscreen_refreshrate, gboffset + 0x1001A354, 6, "\x8B\x81\xC0\x06\x00\x00");
}




// scalefactor table
double scalefactor_table[SCALEFACTOR_COUNT];
static void init_scalefactor_table()
{
    // set default value for scalefactors
    // may be overwritten by other init functions
    int i;
    for (i = 0; i < SCALEFACTOR_COUNT; i++) {
        scalefactor_table[i] = 1.0;
    }
    
    // two special scale factor
    scalefactor_table[SF_IDENTITY] = 1.0;
    scalefactor_table[SF_GAMEFACTOR] = game_scalefactor;
}








// window patch
#define WINDOW_FULLSCREEN 0
#define WINDOW_NORMAL 1
#define WINDOW_NOBORDER 2
static char winname[0x80];

HWND game_hwnd = NULL;
static int window_patch_cfg = -1;
void try_goto_desktop()
{
    // this function is called by fail(), warning()
    // should switch to desktop if necessary
    // NOTE: the graphics patch might not initialized when this function is called
    //       must use static initialize method
    
    if (game_hwnd) {
        if (window_patch_cfg == WINDOW_FULLSCREEN) {
            ShowWindow(game_hwnd, SW_MINIMIZE);
        }
    }
}

static int topmost_ms = 0;
static int topmost_countdown = 0;
static DWORD topmost_create;
static void topmost_gameloop_hook(void *arg)
{
    if (topmost_countdown) {
        if (timeGetTime() - topmost_create >= (unsigned) topmost_ms) {
            SetWindowPos(game_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            topmost_countdown = 0;
        }
    }
}

static ATOM WINAPI RegisterClass_wrapper(WNDCLASS *lpWndClass)
{
    lpWndClass->hIcon = LoadIcon(lpWndClass->hInstance, MAKEINTRESOURCE(101)); // brute force to find this icon ID
    lpWndClass->hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    return RegisterClass(lpWndClass);
}
static HWND WINAPI CreateWindowExA_wrapper(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    if (window_patch_cfg == WINDOW_NORMAL) {
        // x = y = CW_USEDEFAULT;
        x = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 2;
        y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }
    lpWindowName = winname;
    if (topmost_ms != 0) {
        dwExStyle |= WS_EX_TOPMOST;
    }
    game_hwnd = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (topmost_ms > 0) {
        topmost_create = timeGetTime();
        topmost_countdown = 1;
    }
    return game_hwnd;
}
static void getcursorpos_window_hookfunc(void *arg)
{
    POINT *mousept = arg;
    ScreenToClient(game_hwnd, mousept);
}
static void setcursorpos_window_hookfunc(void *arg)
{
    POINT *mousept = arg;
    ClientToScreen(game_hwnd, mousept);
}


// try_refresh_clipcursor() is called by wndproc and extern funtions
// should refresh clipcursor status immediately
// NOTE: the graphics patch (or clipcursor) might not initialized
//       must use static initialize method
static int clipcursor_inited = 0;
static int clipcursor_enabled = 1;
static void clipcursor(int clip)
{
    // check if clipcursor is initialized
    // if not, do nothing
    if (!clipcursor_inited) return;
    
    if (clipcursor_enabled && clip) {
        RECT Rect;
        POINT Point;
        Point.x = Point.y = 0;
        ClientToScreen(game_hwnd, &Point);
        GetClientRect(game_hwnd, &Rect);
        Rect.left = Point.x;
        Rect.top = Point.y;
        Rect.right += Point.x;
        Rect.bottom += Point.y;
        ClipCursor(&Rect);
    } else {
        ClipCursor(NULL);
    }
}
void try_refresh_clipcursor()
{
    if (PAL3_s_bActive) {
        clipcursor(1);
    } else {
        clipcursor(0);
    }
}

static void clipcursor_hook(void *arg)
{
    try_refresh_clipcursor();
}
static void clipcursor_atexit()
{
    clipcursor(0);
}
static void clipcursor_init()
{
    clipcursor_inited = 1;
    add_gameloop_hook(clipcursor_hook);
    add_atexit_hook(clipcursor_atexit);
}






static int confirm_quit()
{
    set_pauseresume(1);
    try_goto_desktop();
    clipcursor(0);
    int ret = MessageBoxW(game_hwnd, wstr_confirmquit_text, wstr_confirmquit_title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) == IDYES;
    set_pauseresume(0);
    return ret;
}

static LRESULT WINAPI DefWindowProcA_wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // debug purpose only
    /*if (Msg == WM_KEYUP && wParam == VK_F11) { // press F11 to toggle UnderWater
        UnderWater_Inst()->m_bEnable ^= 1;
    }*/

    if (Msg == WM_CLOSE) {
        if (!confirm_quit()) return 0;
    }

    if (Msg == WM_KEYUP && wParam == VK_F11) {
        clipcursor_enabled = !clipcursor_enabled;
        try_refresh_clipcursor();
        return 0;
    }
    
    // allow hooks
    LRESULT retvalue;
    if (call_postwndproc_hook(&hWnd, &Msg, &wParam, &lParam, &retvalue)) {
        return retvalue;
    }
    
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

static int early_msgloop = 1;
static LRESULT CALLBACK WndProc_wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // use default procdure when we are in early stage
    if (early_msgloop) {
        if (Msg == WM_CREATE || Msg == WM_DESTROY) {
            goto usepal3;
        } else {
            goto usedefault;
        }
    }
    
    
    if (Msg == WM_SETCURSOR) {
        // show cursor when cursor is in our window but not client area
        if (LOWORD(lParam) != HTCLIENT) goto usedefault;
        
        // show cursor when cursor is in our window but game is not active
        if (!PAL3_s_bActive) goto usedefault;
        
        // otherwise, refresh cursor state
        SetCursor(NULL);
        set_showcursor_state(get_showcursor_state());
        return TRUE;
    }
    
    // pause game when moving window
    if (Msg == WM_SYSCOMMAND && (wParam & 0xFFF0) == SC_MOVE) {
        set_pauseresume(1);
        LRESULT ret = DefWindowProcA(hWnd, Msg, wParam, lParam);
        set_pauseresume(0);
        return ret;
    }
    
    // pause or resume game when active status changed
    if (Msg == WM_ACTIVATE) {
        set_pauseresume(!wParam);
    }
    
    // pause when system menu popup
    switch (Msg) {
        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
            set_pauseresume(1);
            break;
        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
            set_pauseresume(0);
            break;
    }
    
    // allow hooks
    LRESULT retvalue;
    if (call_prewndproc_hook(&hWnd, &Msg, &wParam, &lParam, &retvalue)) {
        return retvalue;
    }
    

usepal3:
    return WndProc(hWnd, Msg, wParam, lParam);
usedefault:
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

static void msgloop_gameloop_hook(void *arg)
{
    // any type of gameloop hook marks the end of early stage
    early_msgloop = 0;
}

static MAKE_THISCALL(void, gbGfxManager_D3D_BuildPresentParamsFromSettings_wrapper, struct gbGfxManager_D3D *this)
{
    // by default, windowed mode ignore vsync settings
    // this wrapper add vsync feature to windowed mode

    gbGfxManager_D3D_BuildPresentParamsFromSettings(this);
    if (this->m_bWindowed) {
        if (this->DrvInfo.waitforverticalblank == 1) {
            this->m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        }
    }
}

static void init_window_patch(int flag)
{
    window_patch_cfg = flag;
    
    if (window_patch_cfg != WINDOW_FULLSCREEN && window_patch_cfg != WINDOW_NORMAL && window_patch_cfg != WINDOW_NOBORDER) {
        fail("unknown window patch configuration.");
    }

    // patch PAL3_s_drvinfo
    if (window_patch_cfg != WINDOW_FULLSCREEN) {
        SIMPLE_PATCH(0x004064DA, "\xC7\x05\xE8\xD6\xBF\x00\x01\x00\x00\x00", "\xC7\x05\xE8\xD6\xBF\x00\x00\x00\x00\x00", 10);
    }

    // change window style
    unsigned newstyle;
    if (window_patch_cfg == WINDOW_FULLSCREEN) {
        // the window style for fullscreen is modified by gbengine after window created
        // 0x96000000 = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
        newstyle = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU;
        memcpy_to_process(gboffset + 0x1011F4B4, &newstyle, 4);
    }
    if (window_patch_cfg == WINDOW_NORMAL) {
        // modify the code which create the window
        newstyle = WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        SIMPLE_PATCH(0x0040652D, "\x00\x00\x08\x96", &newstyle, 4);
        SIMPLE_PATCH(0x00406559, "\x00\x00\x08\x96", &newstyle, 4);
    }
    
    // handle topmost
    if (window_patch_cfg != WINDOW_FULLSCREEN) {
        topmost_ms = get_int_from_configfile("topmost");
    }
    if (topmost_ms != 0) {
        SIMPLE_PATCH(gboffset + 0x1001A9E2, "\x6A\xFE", "\x6A\xFF", 2);
        add_gameloop_hook(topmost_gameloop_hook);
    }

    // hook CreateWindowEx
    make_call6(0x004063A7, RegisterClass_wrapper);
    make_call6(0x00406564, CreateWindowExA_wrapper);

    // modify window title
    char *winname_mbcs = cs2cs_alloc(get_string_from_configfile("windowtitle"), CP_UTF8, CP_ACP);
    strncpy(winname, winname_mbcs, sizeof(winname));
    free(winname_mbcs);
    winname[sizeof(winname) - 1] = 0;
    unsigned titleaddr = TOUINT(winname);
    SIMPLE_PATCH(0x00541925, "\xC4\x39\x58\x00", &titleaddr, 4);
    
    // cursor hook
    add_getcursorpos_hook(getcursorpos_window_hookfunc);
    add_setcursorpos_hook(setcursorpos_window_hookfunc);
    
    // clip cursor
    if (!GET_PATCHSET_FLAG(testcombat) && get_int_from_configfile("clipcursor")) {
        clipcursor_init();
    }
    
    // patch gbGfxManager_D3D::m_bClipCursorWhenFullscreen
    // the game will use our clip cursor framework
    SIMPLE_PATCH(gboffset + 0x10018A1B, "\xC6\x86\x4A\x06\x00\x00\x01", "\xC6\x86\x4A\x06\x00\x00\x00", 7);

    // hook WndProc
    make_pointer(0x0040637F + 0x4, WndProc_wrapper);
    
    // hook DefWindowProc
    make_call6(0x00404F8D, DefWindowProcA_wrapper);
    
    // add early message loop flag resetting function
    add_gameloop_hook(msgloop_gameloop_hook);

    // click X to quit game
    if (get_int_from_configfile("xquit")) {
        SIMPLE_PATCH_NOP(0x00404F57, "\x74\x52", 2);
    }
    
    // hook gbGfxManager_D3D_BuildPresentParamsFromSettings()
    INIT_WRAPPER_CALL(gbGfxManager_D3D_BuildPresentParamsFromSettings_wrapper, { gboffset + 0x1001A828 });
}




int skipupdate_state = 0;
static MAKE_ASMPATCH(check_skipupdate)
{
    R_EAX = M_DWORD(0x005833B8);
    if (skipupdate_state) {
        skipupdate_state = 1;
        R_EAX = 0;
    }
}
static MAKE_ASMPATCH(is_editorwindow)
{
    if (M_BYTE(0x00BFDA90) == 1) { // testcombat
        LINK_CALL(0x00513380);
    } else {
        R_EAX = 0;
        LINK_CALL(0x0051338D);
    }
}
static void init_skipupdate_patch()
{
    INIT_ASMPATCH(check_skipupdate, 0x00541B7F, 5, "\xA1\xB8\x33\x58\x00");
    INIT_ASMPATCH(is_editorwindow, 0x00541BA6, 5, "\xE8\xD5\x17\xFD\xFF");
}





double str2scalefactor(const char *str)
{
    int bound_flag = 1;
    if (*str == '!') { str++; bound_flag = 0; }
    double ret;
    if (stricmp(str, "large") == 0) {
        ret = game_scalefactor;
    } else if (stricmp(str, "medium") == 0) {
        ret = game_scalefactor * 0.75;
    } else if (stricmp(str, "small") == 0) {
        ret = game_scalefactor * 0.5;
    } else if (stricmp(str, "none") == 0) {
        ret = 1.0;
    } else if (strnicmp(str, "abs", 3) == 0) {
        ret = str2double(str + 3);
    } else if (strnicmp(str, "rel", 3) == 0) {
        ret = game_scalefactor * str2double(str + 3);
    } else {
        fail("unknown scalefactor string %s.", str);
    }
    return bound_flag ? fbound(ret, 1.0, game_scalefactor) : ret;
}




static void loading_splash()
{
    IDirect3DDevice9_Clear(GB_GfxMgr->m_pd3dDevice, 0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
    IDirect3DDevice9_BeginScene(GB_GfxMgr->m_pd3dDevice);
    
    int fontsize = floor(16 * game_scalefactor + eps);
    ID3DXFont *pFont;
    if (FAILED(myD3DXCreateFontW(GB_GfxMgr->m_pd3dDevice, fontsize, 0, FW_BOLD, 0, FALSE, defaultfont_charset, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, wstr_defaultfont, &pFont))) {
        pFont = NULL;
    }
    if (pFont) {
        RECT rc;
        int padding = floor(20 * game_scalefactor + eps);
        set_rect(&rc, 0, PAL3_s_drvinfo.height - padding - fontsize, PAL3_s_drvinfo.width - padding, 0);
        myID3DXFont_DrawTextW(pFont, NULL, wstr_gameloading, -1, &rc, DT_NOCLIP | DT_RIGHT, 0xFFFFFFFF);
        myID3DXFont_Release(pFont);
        pFont = NULL;
    }
    
    IDirect3DDevice9_EndScene(GB_GfxMgr->m_pd3dDevice);
    IDirect3DDevice9_Present(GB_GfxMgr->m_pd3dDevice, NULL, NULL, NULL, NULL);
}
static void add_loading_splash()
{
    add_postd3dcreate_hook(loading_splash);
}

static void init_resolution_and_window_patch()
{
    // read config
    int window_cfg = get_int_from_configfile("game_windowed");
    const char *resolution_cfg = get_string_from_configfile("game_resolution");
    
    // adjust config
    if (GET_PATCHSET_FLAG(testcombat)) {
        if (strcmp(resolution_cfg, "current") == 0) resolution_cfg = "800x600";
        window_cfg = WINDOW_NOBORDER;
    } else {
        if (window_cfg == WINDOW_NOBORDER) resolution_cfg = "current";
    }
    if (strcmp(resolution_cfg, "current") == 0 && window_cfg == WINDOW_NORMAL) resolution_cfg = "800x600";
    
    // init patches
    init_window_patch(window_cfg);
    patch_resolution_config(resolution_cfg);
    init_skipupdate_patch();
}


static char scursor_flag;
static char scursor_PAL3_HasCommandLineParam(char *s)
{
    return scursor_flag;
}
static void init_softcursor_patch()
{
    scursor_flag = !!get_int_from_configfile("softcursor");
    make_call(0x004066AD, scursor_PAL3_HasCommandLineParam);
}


static struct gbGfxManager *gbCreateGraphManager_wrapper(struct gbGfxDriverInfo *pinfo)
{
    struct gbGfxManager *pgfx = gbCreateGraphManager(pinfo);
    if (!pgfx) {
        try_goto_desktop();
        
        const wchar_t *badtools = detect_badtools();
    	if (badtools) {
            MessageBoxW_format(game_hwnd, wstr_nogfxmgr_badtools_text, wstr_nogfxmgr_title, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND, badtools);
        } else {
            MessageBoxW(game_hwnd, wstr_nogfxmgr_text, wstr_nogfxmgr_title, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
        }
    }
    return pgfx;
}
static void init_gfxmgr_checker()
{
    make_call6(0x00405013, gbCreateGraphManager_wrapper);
}


MAKE_PATCHSET(graphicspatch)
{
    patch_refreshrate_config(get_string_from_configfile("game_refreshrate"));
    patch_depth_buffer_config(get_string_from_configfile("game_zbufferbits"));
    patch_multisample_config(get_string_from_configfile("game_multisample"));
    init_resolution_and_window_patch();
    init_scalefactor_table();
    fpslimit_init();
    add_loading_splash();
    init_softcursor_patch();
    init_gfxmgr_checker();
}
