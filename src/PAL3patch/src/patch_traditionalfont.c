#include "common.h"

static int (WINAPI *my_AddFontResourceExW)(LPCWSTR, DWORD, PVOID) = NULL;
static BOOL (WINAPI *my_RemoveFontResourceExW)(LPCWSTR, DWORD, PVOID) = NULL;

static int font_count;

#define MY_FR_PRIVATE 0x10

static void remove_font_hook(void)
{
    if (font_count > 0 && my_RemoveFontResourceExW) {
        my_RemoveFontResourceExW(wstr_defaultfont_filename, MY_FR_PRIVATE, 0);
    }
}

MAKE_PATCHSET(traditionalfont)
{
    // import functions
    HMODULE hGDI32 = LoadLibrary("GDI32.DLL");
    if (hGDI32) {
        my_AddFontResourceExW = TOPTR(GetProcAddress(hGDI32, "AddFontResourceExW"));
        my_RemoveFontResourceExW = TOPTR(GetProcAddress(hGDI32, "RemoveFontResourceExW"));
    }
    
    // load font
    font_count = 0;
    if (my_AddFontResourceExW) {
        font_count = my_AddFontResourceExW(wstr_defaultfont_filename, MY_FR_PRIVATE, 0);
    }
    if (font_count == 0) {
        warning("unable to add font resource '%ls'.", wstr_defaultfont_filename);
    }
    
    // add atexit hook to remove font when exiting
    add_atexit_hook(remove_font_hook);
}
