#include "common.h"

static int window_patch_cfg;
#define WINDOW_NORMAL 1
#define WINDOW_NOBORDER 2

static char winname[0x80];
static HWND gamehwnd;

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
    return MessageBoxW(gamehwnd, wstr_confirmquit_text, wstr_confirmquit_title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES;
}

static LRESULT WINAPI DefWindowProcA_wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == WM_SYSCOMMAND && wParam == SC_CLOSE && !confirm_quit()) return 0;
    if (Msg == WM_KEYUP && wParam == VK_F12) { clipcursor_enabled ^= 1; return 0; }
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

MAKE_PATCHSET(windowed)
{
    window_patch_cfg = flag;
    if (window_patch_cfg != WINDOW_NORMAL && window_patch_cfg != WINDOW_NOBORDER) {
        fail("unknown window patch configuration.");
    }

    SIMPLE_PATCH(0x004064DA, "\xC7\x05\xE8\xD6\xBF\x00\x01\x00\x00\x00", "\xC7\x05\xE8\xD6\xBF\x00\x00\x00\x00\x00", 10);

    // change window style
    if (window_patch_cfg == WINDOW_NORMAL) {
        unsigned newstyle = WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
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
}
