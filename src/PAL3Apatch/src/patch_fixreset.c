#include "common.h"


static LRESULT WINAPI SendMessageA_wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret;
    if (Msg == 0x590) call_onlostdevice_hooks();
    ret = SendMessageA(hWnd, Msg, wParam, lParam);
    if (Msg == 0x591) call_onresetdevice_hooks();
    return ret;
}

static MAKE_ASMPATCH(fixreset_gbGfxManager_D3D_BeginScene_patch)
{
    gbGfxManager_D3D_EnsureCooperativeLevel(TOPTR(R_ECX), 0);
    
    PUSH_DWORD(R_EBX); // oldcode
    PUSH_DWORD(R_ESI);
    PUSH_DWORD(R_EDI);
    R_ESI = R_ECX;
}

static MAKE_ASMPATCH(retryreset)
{
    if (R_EAX >> 31) {
        if (MessageBox(NULL, "Reset() failed, retry?\n\npress ENTER to retry.\npress ESC to exit game.", "PAL3Apatch", MB_RETRYCANCEL | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND | MB_SYSTEMMODAL) == IDCANCEL) {
            RETNADDR = gboffset + 0x1001A5AE; // oldcode
        } else {
            RETNADDR = gboffset + 0x1001A4B6; // try Reset() again
        }
    }
}

MAKE_PATCHSET(fixreset)
{
    // patch gbGfxManager_D3D::BeginScene
    INIT_ASMPATCH(fixreset_gbGfxManager_D3D_BeginScene_patch, gboffset + 0x100186D6, 5, "\x53\x56\x57\x8B\xF1");

    // install DeviceLost and ResetDevice hooks
    char codebuf[6] = "\xBB\x00\x00\x00\x00\x90";
    void *funcptr = SendMessageA_wrapper;
    memcpy(codebuf + 1, &funcptr, 4);
    memcpy_to_process(gboffset + 0x1001A487, codebuf, sizeof(codebuf));

    // patch Reset3DEnvironment
    INIT_ASMPATCH(retryreset, gboffset + 0x1001A4CB, 6, "\x0F\x8C\xDD\x00\x00\x00");
}
