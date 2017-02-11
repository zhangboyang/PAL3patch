#include "common.h"

#ifdef DYNLINK_D3DX9_AT_RUNTIME

/*
    PAL3's gbengine.dll uses D3DX9 from summer 2003 SDK
    the D3DX_SDK_VERSION is 21 in this SDK, so the DLL name is d3dx9_21.dll
    in fact, Microsoft has never release such DLL, this DLL is compiled by myself
*/
#define D3DX9_DLL "d3dx9_" TOSTR(D3DX_SDK_VERSION) ".dll"

#define IMPORT_D3DX9_FUNCPTR(func) (D3DXFUNC(func) = (void *) GetProcAddress_check(hD3DX9, TOSTR(func)))

static HMODULE hD3DX9;

void d3dx9_dynlink()
{
    hD3DX9 = LoadLibrary_check(D3DX9_DLL);
    
    //IMPORT_D3DX9_FUNCPTR(D3DXSaveTextureToFileA);
    IMPORT_D3DX9_FUNCPTR(D3DXCreateFontW);
    IMPORT_D3DX9_FUNCPTR(D3DXCreateSprite);
    IMPORT_D3DX9_FUNCPTR(D3DXGetImageInfoFromFileInMemory);
    IMPORT_D3DX9_FUNCPTR(D3DXLoadSurfaceFromFileInMemory);
    IMPORT_D3DX9_FUNCPTR(D3DXFillTexture);    
}

#endif
