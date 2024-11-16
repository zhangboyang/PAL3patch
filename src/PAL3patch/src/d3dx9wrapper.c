#include "common.h"

typedef struct {
    const void *const *vtbl;
    ID3DXEffect43 *p;
    int state;
} proxyID3DXEffect;

static ULONG STDMETHODCALLTYPE proxyID3DXEffect_Release(proxyID3DXEffect *this)
{
    ULONG ret = ID3DXEffect_Release(this->p);
    free(this);
    return ret;
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_Begin(proxyID3DXEffect *this, UINT *pPasses, DWORD Flags)
{
    this->state = 0;
    return ID3DXEffect_Begin(this->p, pPasses, Flags);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_Pass(proxyID3DXEffect *this, UINT Pass)
{
    if (this->state) ID3DXEffect_EndPass(this->p);
    this->state = 1;
    return ID3DXEffect_BeginPass(this->p, Pass);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_End(proxyID3DXEffect *this)
{
    if (this->state) ID3DXEffect_EndPass(this->p);
    this->state = 0;
    return ID3DXEffect_End(this->p);
}
static D3DXHANDLE STDMETHODCALLTYPE proxyID3DXEffect_GetParameterByName(proxyID3DXEffect *this, D3DXHANDLE hParameter, LPCSTR pName)
{
    return ID3DXEffect_GetParameterByName(this->p, hParameter, pName);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_SetValue(proxyID3DXEffect *this, D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes)
{
    HRESULT ret = ID3DXEffect_SetValue(this->p, hParameter, pData, Bytes);
    if (ret != D3D_OK) return ret;
    return this->state ? ID3DXEffect_CommitChanges(this->p) : ret;
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_GetValue(proxyID3DXEffect *this, D3DXHANDLE hParameter, LPVOID pData, UINT Bytes)
{
    return ID3DXEffect_GetValue(this->p, hParameter, pData, Bytes);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_OnLostDevice(proxyID3DXEffect *this)
{
    return ID3DXEffect_OnLostDevice(this->p);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_OnResetDevice(proxyID3DXEffect *this)
{
    return ID3DXEffect_OnResetDevice(this->p);
}

static const void *const proxyID3DXEffectVtbl[] = {
    NULL,
    NULL,
    proxyID3DXEffect_Release,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    proxyID3DXEffect_GetParameterByName,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    proxyID3DXEffect_SetValue,
    proxyID3DXEffect_GetValue,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    proxyID3DXEffect_Begin,
    proxyID3DXEffect_Pass,
    proxyID3DXEffect_End,
    NULL,
    proxyID3DXEffect_OnLostDevice,
    proxyID3DXEffect_OnResetDevice,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static HRESULT (WINAPI *D3DXCreateEffect43)(LPDIRECT3DDEVICE9, LPCVOID, UINT, const D3DXMACRO *, LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT *, LPD3DXBUFFER *);

static HRESULT WINAPI proxyD3DXCreateEffect(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataLen, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors)
{
    Flags |= D3DXFX43_LARGEADDRESSAWARE | D3DXFX43_NOT_CLONEABLE;
    HRESULT ret = D3DXCreateEffect43(pDevice, pSrcData, SrcDataLen, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    if (ret != D3D_OK) return ret;
    proxyID3DXEffect *this = malloc(sizeof(proxyID3DXEffect));
    this->vtbl = proxyID3DXEffectVtbl;
    this->p = TOPTR(*ppEffect);
    this->state = 0;
    *ppEffect = TOPTR(this);
    return D3D_OK;
}

HMODULE d3dx9_43 = NULL;

HRESULT (WINAPI *myD3DXCreateFontW)(LPDIRECT3DDEVICE9, UINT, UINT, UINT, UINT, BOOL, DWORD, DWORD, DWORD, DWORD, LPCWSTR, LPD3DXFONT *);
HRESULT (WINAPI *myD3DXCreateSprite)(LPDIRECT3DDEVICE9, LPD3DXSPRITE *);
HRESULT (WINAPI *myD3DXGetImageInfoFromFileInMemory)(LPCVOID, UINT, D3DXIMAGE_INFO *);
HRESULT (WINAPI *myD3DXLoadSurfaceFromFileInMemory)(LPDIRECT3DSURFACE9, CONST PALETTEENTRY *, CONST RECT *, LPCVOID, UINT, CONST RECT *, DWORD, D3DCOLOR, D3DXIMAGE_INFO *);
HRESULT (WINAPI *myD3DXFillTexture)(LPDIRECT3DTEXTURE9, LPD3DXFILL2D, LPVOID);
HRESULT (WINAPI *myD3DXSaveSurfaceToFileA)(LPCTSTR, D3DXIMAGE_FILEFORMAT, LPDIRECT3DSURFACE9, const PALETTEENTRY *, const RECT *);

unsigned gbD3DXCreateEffect;

static void fuse_d3dx9_43()
{
#define FUNC_D3DX9_43(f) TOPTR(GetProcAddress_check(d3dx9_43, TOSTR(f)))
#define IMPORT_D3DX9_43(f) (CONCAT(my, f) = FUNC_D3DX9_43(f))

    IMPORT_D3DX9_43(D3DXCreateFontW); // d3dx9_43: INT Height, d3dx9_21: UINT Height
    IMPORT_D3DX9_43(D3DXCreateSprite);
    IMPORT_D3DX9_43(D3DXGetImageInfoFromFileInMemory);
    IMPORT_D3DX9_43(D3DXLoadSurfaceFromFileInMemory);
    IMPORT_D3DX9_43(D3DXFillTexture);
    IMPORT_D3DX9_43(D3DXSaveSurfaceToFileA);
    
    D3DXCreateEffect43 = FUNC_D3DX9_43(D3DXCreateEffect);
    gbD3DXCreateEffect = TOUINT(proxyD3DXCreateEffect);
    
    INIT_WRAPPER_CALL(myD3DXGetImageInfoFromFileInMemory, {
        gboffset + 0x1001B8E7,
        gboffset + 0x1001C030,
    });
    INIT_WRAPPER_CALL(myD3DXLoadSurfaceFromFileInMemory, {
        gboffset + 0x1001B924,
    });
    INIT_WRAPPER_CALL(FUNC_D3DX9_43(D3DXCreateTextureFromFileInMemoryEx), {
        gboffset + 0x1001C0F4,
        gboffset + 0x1001C11E,
        gboffset + 0x1001C14A,
    });
    INIT_WRAPPER_CALL(FUNC_D3DX9_43(D3DXCreateCubeTextureFromFileInMemoryEx), {
        gboffset + 0x1001C0AE,
    });
}

static void fuse_builtin_d3dx9()
{
#define IMPORT_BUILTIN_D3DX9(f) (CONCAT(my, f) = f)

    IMPORT_BUILTIN_D3DX9(D3DXCreateFontW);
    IMPORT_BUILTIN_D3DX9(D3DXCreateSprite);
    IMPORT_BUILTIN_D3DX9(D3DXGetImageInfoFromFileInMemory);
    IMPORT_BUILTIN_D3DX9(D3DXLoadSurfaceFromFileInMemory);
    IMPORT_BUILTIN_D3DX9(D3DXFillTexture);
    IMPORT_BUILTIN_D3DX9(D3DXSaveSurfaceToFileA);
    
    gbD3DXCreateEffect = gboffset + 0x1003439A;
}

void fuse_d3dx9_wrapper()
{
    if (d3dx9_43) {
        d3d_sdk_version = 32;
        fuse_d3dx9_43();
    } else {
        fuse_builtin_d3dx9();
    }
}

void init_d3dx9_wrapper()
{
    if (is_laa()) {
        HMODULE hD3DX9 = LoadLibraryA(D3DX9_43_DLL);
        if (hD3DX9 && LoadLibraryA(D3DCOMPILER_43_DLL)) {
            d3dx9_43 = hD3DX9;
        } else {
            if (hD3DX9) FreeLibrary(hD3DX9);
            warning("LargeAddressAware enabled, but failed to load both '" D3DX9_43_DLL "' and '" D3DCOMPILER_43_DLL "', game may crash.");
        }
    }
}
