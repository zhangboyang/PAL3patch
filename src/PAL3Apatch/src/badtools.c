#include "common.h"

static const wchar_t *detect_dxwnd(void)
{
    static int has_dxwnd = 0;
  	if (!has_dxwnd) {
        HMODULE dxwnd_dll = GetModuleHandle("dxwnd.dll");
        has_dxwnd = !!dxwnd_dll;
    }
    return has_dxwnd ? wstr_badtool_dxwnd : NULL;
}

static const wchar_t *detect_d3dwindower(void)
{
    static int has_d3dwindower = 0;
    if (!has_d3dwindower) {
        HMODULE d3dhook_dll = GetModuleHandle("d3dhook.dll");
        has_d3dwindower = !!(d3dhook_dll && GetProcAddress(d3dhook_dll, "@Madcodehook@HookAPI$qqspct1pvrpvui"));
    }
    return has_d3dwindower ? wstr_badtool_d3dwindower : NULL;
}

const wchar_t *detect_badtools(void)
{
	static const wchar_t *(*const detectors[])(void) = {
        detect_dxwnd,
        detect_d3dwindower,
    };

    static wchar_t *badtools = NULL;
    unsigned i;
	struct wstr buf;
    wstr_ctor(&buf);
	
	for (i = 0; i < sizeof(detectors) / sizeof(detectors[0]); i++) {
        const wchar_t *result = detectors[i]();
        if (result) {
            wstr_wcscat(&buf, L"    ");
            wstr_wcscat(&buf, result);
            wstr_wcscat(&buf, L"\n");
        }
    }
	
	if (!wstr_empty(&buf)) {
        free(badtools);
        badtools = wcsdup(wstr_getwcs(&buf));
    }
    
	wstr_dtor(&buf);
	return badtools;
}

void check_badtools(void)
{
    const wchar_t *badtools = detect_badtools();
	if (badtools) {
		if (MessageBoxW_format(NULL, wstr_havebadtool_text, wstr_havebadtool_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND, badtools) != IDYES) {
            die(0);
		}
	}
}
