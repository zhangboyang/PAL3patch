#include "common.h"

int d3d_sdk_version = 31;

static IDirect3D9 *(WINAPI *pDirect3DCreate9)(UINT) = NULL;

static IDirect3D9 *WINAPI myDirect3DCreate9(UINT SDKVersion)
{
    fuse_d3dx9_wrapper();
    return pDirect3DCreate9(d3d_sdk_version);
}

void init_d3d9_wrapper()
{
    const char *d3d9dll = get_string_from_configfile("d3d9");
    HMODULE hD3D9 = LoadLibraryA(d3d9dll);
    if (hD3D9) {
        pDirect3DCreate9 = TOPTR(GetProcAddress(hD3D9, "Direct3DCreate9"));
    }
    if (!pDirect3DCreate9) {
        warning("can't import Direct3DCreate9 from '%s'.", d3d9dll);
        pDirect3DCreate9 = TOPTR(M_DWORD(gboffset + 0x100F5230));
    }
    make_jmp(gboffset + 0x10032CB0, myDirect3DCreate9);
}
