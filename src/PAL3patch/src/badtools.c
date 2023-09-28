#include "common.h"

static const wchar_t *detect_dxwnd(void)
{
    static int has_dxwnd = 0;
  	if (!has_dxwnd) {
        has_dxwnd = !!GetModuleHandle("dxwnd.dll");
    }
    return has_dxwnd ? wstr_badtool_dxwnd : NULL;
}

const wchar_t *detect_badtools(void)
{
	static const wchar_t *(*const detectors[])(void) = {
        detect_dxwnd,
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
