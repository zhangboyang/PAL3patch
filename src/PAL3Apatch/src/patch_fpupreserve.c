#include "common.h"

static HRESULT STDMETHODCALLTYPE IDirect3D9_CreateDevice_wrapper(IDirect3D9 *this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    BehaviorFlags |= D3DCREATE_FPU_PRESERVE;
    HRESULT ret = IDirect3D9_CreateDevice(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    _setdefaultprecision();
    return ret;
}

static MAKE_ASMPATCH(fpupreserve)
{
    PUSH_DWORD(R_ECX);
    PUSH_DWORD(R_EAX);
    LINK_CALL(TOUINT(IDirect3D9_CreateDevice_wrapper));
}

MAKE_PATCHSET(fpupreserve)
{
    INIT_ASMPATCH(fpupreserve, gboffset + 0x1001A1C0, 5, "\x51\x50\xFF\x57\x40");
}
