#include "common.h"

#ifdef DYNLINK_D3DX9_AT_RUNTIME

#define DECLARE_D3DX9_FUNCPTR(funcptr) typeof(funcptr) funcptr
#define IMPORT_D3DX9_FUNCPTR(funcptr) ((funcptr) = (void *) GetProcAddress_safe(hD3DX9, TOSTR(funcptr)))

//DECLARE_D3DX9_FUNCPTR(D3DXSaveTextureToFileA);
DECLARE_D3DX9_FUNCPTR(D3DXCreateFontW);
DECLARE_D3DX9_FUNCPTR(D3DXCreateSprite);
DECLARE_D3DX9_FUNCPTR(D3DXGetImageInfoFromFileInMemory);
DECLARE_D3DX9_FUNCPTR(D3DXLoadSurfaceFromFileInMemory);
DECLARE_D3DX9_FUNCPTR(D3DXFillTexture);

static HMODULE hD3DX9;

void d3dx9_dynlink()
{
    hD3DX9 = LoadLibrary_safe(D3DX9_DLL);
    
    //IMPORT_D3DX9_FUNCPTR(D3DXSaveTextureToFileA);
    IMPORT_D3DX9_FUNCPTR(D3DXCreateFontW);
    IMPORT_D3DX9_FUNCPTR(D3DXCreateSprite);
    IMPORT_D3DX9_FUNCPTR(D3DXGetImageInfoFromFileInMemory);
    IMPORT_D3DX9_FUNCPTR(D3DXLoadSurfaceFromFileInMemory);
    IMPORT_D3DX9_FUNCPTR(D3DXFillTexture);    
}

#endif
