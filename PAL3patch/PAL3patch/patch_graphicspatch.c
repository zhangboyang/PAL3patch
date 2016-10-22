#include "common.h"

// game resolution
int game_width, game_height;
fRECT game_frect, game_frect_43, game_frect_original;
double game_scalefactor;




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
    for (i = 0; i < this->m_NumEntries; i++) {
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
        R_EAX = TOUINT(CArrayList_GetPtr(this, this->m_NumEntries - 1));
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
                // there no need to use stencil buffer
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X8, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24X4S4, NULL);
                if (!result) result = CArrayList_FindInt(this, D3DFMT_D24S8, NULL);
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
    zbuf_flag = (stricmp(cfgstr, "max") == 0) ? INT_MAX : str2int(cfgstr);
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
        id = tl->m_NumEntries - 1;
        *tret = *(int *) CArrayList_GetPtr(tl, id);
    } else if (ms_tflag >= msTypeArrayCount) {
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
        q = q - 1;
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
    ms_tflag = (stricmp(buf, "max") == 0) ? INT_MAX : str2int(buf);
    ms_qflag = (stricmp(ptr, "max") == 0) ? INT_MAX : str2int(ptr);
    if (ms_tflag || ms_qflag) {
        INIT_ASMPATCH(multisample_windowed, gboffset + 0x10019E2E , 16, "\xC7\x87\x98\x06\x00\x00\x00\x00\x00\x00\x89\x87\x94\x06\x00\x00");
        INIT_ASMPATCH(multisample_fullscreen, gboffset + 0x1001A15B, 12, "\x89\x86\xCC\x06\x00\x00\x89\xBE\xD0\x06\x00\x00");
    }
}








// resolution patch
static char __fastcall Readn(void *this, int dummy, char *appname, char *keyname, int *ret, int defvalue)
{
    if (strcmp(keyname, "width") == 0) { *ret = game_width; return 1; }
    if (strcmp(keyname, "height") == 0) { *ret = game_height; return 1; }
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
    
    set_frect_ltwh(&game_frect_original, 0, 0, GAME_WIDTH_ORG, GAME_HEIGHT_ORG);
    set_frect_ltwh(&game_frect, 0, 0, game_width, game_height);
    get_ratio_frect(&game_frect_43, &game_frect, 4.0, 3.0);
    game_scalefactor = get_frect_min_scalefactor(&game_frect_43, &game_frect_original);
    make_call(0x00406436, Readn);
    make_call(0x00406453, Readn);
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
static HWND gamehwnd;
static int window_patch_cfg;

static ATOM WINAPI RegisterClass_wrapper(WNDCLASS *lpWndClass)
{
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
    gamehwnd = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    return gamehwnd;
}
static void getcursorpos_window_hookfunc()
{
    if (!getcursorpos_hook_ret) return;
    ScreenToClient(gamehwnd, getcursorpos_hook_lppoint);
}
static BOOL WINAPI SetCursorPos_wrapper(int X, int Y)
{
    POINT cpos;
    cpos.x = X;
    cpos.y = Y;
    ClientToScreen(gamehwnd, &cpos);
    return SetCursorPos(cpos.x, cpos.y);
}
static int clipcursor_enabled = 1;
static void clipcursor(int flag)
{
    if (clipcursor_enabled && flag) {
        RECT Rect;
        POINT Point;
        Point.x = Point.y = 0;
        ClientToScreen(gamehwnd, &Point);
        GetClientRect(gamehwnd, &Rect);
        Rect.left = Point.x;
        Rect.top = Point.y;
        Rect.right += Point.x;
        Rect.bottom += Point.y;
        ClipCursor(&Rect);
    } else {
        ClipCursor(NULL);
    }
}
static void clipcursor_atexit()
{
    clipcursor(0);
}
static void clipcursor_hook()
{
    if (is_window_active) {
        clipcursor(1);
    } else {
        clipcursor(0);
    }
}
static int confirm_quit()
{
    ClipCursor(NULL);
    return MessageBoxW(gamehwnd, wstr_confirmquit_text, wstr_confirmquit_title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) == IDYES;
}
static LRESULT WINAPI DefWindowProcA_wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == WM_CLOSE) {
        if (window_patch_cfg == WINDOW_FULLSCREEN) ShowWindow(gamehwnd, SW_MINIMIZE);
        if (!confirm_quit()) return 0;
    }
    if (Msg == WM_KEYUP && wParam == VK_F12) { clipcursor_enabled ^= 1; return 0; }
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}
static void __fastcall gbGfxManager_D3D_BuildPresentParamsFromSettings_wrapper(struct gbGfxManager_D3D *this, int dummy)
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

    // patch gfxdrvinfo
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
    

    // hook CreateWindowEx
    make_branch(0x004063A7, 0xE8, RegisterClass_wrapper, 6);
    make_branch(0x00406564, 0xE8, CreateWindowExA_wrapper, 6);

    // modify window title
    strncpy(winname, get_string_from_configfile("windowtitle"), sizeof(winname));
    winname[sizeof(winname) - 1] = 0;
    unsigned titleaddr = TOUINT(winname);
    SIMPLE_PATCH(0x00541925, "\xC4\x39\x58\x00", &titleaddr, 4);
    
    // cursor hook
    add_getcursorpos_hook(getcursorpos_window_hookfunc);
    // make_branch(0x00404F2B, 0xE8, GetCursorPos_wrapper, 6);
    make_jmp(0x00402290, SetCursorPos_wrapper);
    
    // clip cursor
    if (get_int_from_configfile("clipcursor")) {
        add_gameloop_hook(clipcursor_hook);
        add_atexit_hook(clipcursor_atexit);
    }
    
    // hook DefWindowProc
    make_branch(0x00404F8D, 0xE8, DefWindowProcA_wrapper, 6);
    
    // click X to quit game
    if (get_int_from_configfile("xquit")) {
        SIMPLE_PATCH_NOP(0x00404F57, "\x74\x52", 2);
    }
    
    // hook gbGfxManager_D3D_BuildPresentParamsFromSettings()
    make_call(0x1001A828, gbGfxManager_D3D_BuildPresentParamsFromSettings_wrapper);
}












MAKE_PATCHSET(graphicspatch)
{
    patch_depth_buffer_config(get_string_from_configfile("game_zbufferbits"));
    patch_multisample_config(get_string_from_configfile("game_multisample"));
    patch_resolution_config(get_string_from_configfile("game_resolution"));
    init_scalefactor_table();
    init_window_patch(get_int_from_configfile("game_windowed"));
}
