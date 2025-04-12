#include "common.h"

int d3d_sdk_version;
HMODULE d3dx9_43 = NULL;

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
    if (this->state >= 0) return D3DERR_INVALIDCALL;
    this->state = 0;
    return ID3DXEffect_Begin(this->p, pPasses, Flags);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_Pass(proxyID3DXEffect *this, UINT Pass)
{
    if (this->state > 0) ID3DXEffect_EndPass(this->p);
    this->state = 1;
    return ID3DXEffect_BeginPass(this->p, Pass);
}
static HRESULT STDMETHODCALLTYPE proxyID3DXEffect_End(proxyID3DXEffect *this)
{
    if (this->state < 0) return E_FAIL;
    if (this->state > 0) ID3DXEffect_EndPass(this->p);
    this->state = -1;
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
    return this->state > 0 ? ID3DXEffect_CommitChanges(this->p) : ret;
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
    this->state = -1;
    *ppEffect = TOPTR(this);
    return D3D_OK;
}

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
        gboffset + 0x1001B29C,
        gboffset + 0x1001B9FE,
    });
    INIT_WRAPPER_CALL(myD3DXLoadSurfaceFromFileInMemory, {
        gboffset + 0x1001B2D7,
    });
    INIT_WRAPPER_CALL(FUNC_D3DX9_43(D3DXCreateTextureFromFileInMemoryEx), {
        gboffset + 0x1001BAC0,
        gboffset + 0x1001BAE9,
        gboffset + 0x1001BB17,
    });
    INIT_WRAPPER_CALL(FUNC_D3DX9_43(D3DXCreateCubeTextureFromFileInMemoryEx), {
        gboffset + 0x1001BA7B,
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
    
    gbD3DXCreateEffect = gboffset + 0x10031BA2;
}

static void fuse_d3dx9()
{
    if (d3dx9_43) {
        fuse_d3dx9_43();
    } else {
        fuse_builtin_d3dx9();
    }
}

static int load_d3dx9(int silent)
{
    if (d3d_sdk_version >= 32 && !d3dx9_43) {
        HMODULE hD3DX9 = LoadLibraryA(D3DX9_43_DLL);
        if (hD3DX9 && LoadLibraryA(D3DCOMPILER_43_DLL)) {
            d3dx9_43 = hD3DX9;
        } else {
            if (!silent) fail("failed to load both '" D3DX9_43_DLL "' and '" D3DCOMPILER_43_DLL "'.");
            return 0;
        }
    }
    return 1;
}

static IDirect3D9 *(WINAPI *pDirect3DCreate9)(UINT) = NULL;

static IDirect3D9 *WINAPI myDirect3DCreate9(UINT SDKVersion)
{
    load_d3dx9(0);
    fuse_d3dx9();
    return pDirect3DCreate9(d3d_sdk_version);
}

static int parse_d3d9_config(const char *config, int *laa, int *ver, const char **dll)
{
    const char *x = config;
    const char *y = strchr(x, ',');
    if (!y) return 0;
    y++;
    const char *z = strchr(y, ',');
    if (!z) return 0;
    z++;
    if (sscanf(x, "%d", laa) != 1) return 0;
    if (sscanf(y, "%d", ver) != 1) return 0;
    *dll = z;
    return 1;
}
static int load_d3d9(const char *config, int silent)
{
    int laa;
    const char *dll;
    if (!parse_d3d9_config(config, &laa, &d3d_sdk_version, &dll)) {
        if (!silent) fail("can't parse d3d9 config.");
        return 0;
    }
    if (laa && d3d_sdk_version < 32) {
        if (!silent) fail("D3DX of Direct3D 9.0b is not compatible with LargeAddressAware.");
        return 0;
    }
    fork_laa(laa);
    HMODULE hD3D9 = LoadLibraryA(dll);
    if (!hD3D9) {
        if (!silent) fail("can't load '%s'.", dll);
        return 0;
    }
    pDirect3DCreate9 = TOPTR(GetProcAddress(hD3D9, "Direct3DCreate9"));
    if (!pDirect3DCreate9) {
        if (!silent) fail("can't import Direct3DCreate9 from '%s'.", dll);
        return 0;
    }
    return load_d3dx9(silent);
}

int prepare_d3denum(const char *config)
{
    return load_d3d9(config, 1);
}
int run_d3denum(FILE *fp)
{
    IDirect3D9 *pD3D = pDirect3DCreate9(d3d_sdk_version);
    if (!pD3D) return 0;
    extern int d3denum(FILE *fp, IDirect3D9 *pD3D);
    return d3denum(fp, pD3D);
}

void prepare_d3d9_alternative()
{
    load_d3d9(get_string_from_configfile("d3d9"), 0);
}
void init_d3d9_alternative()
{
    make_jmp(gboffset + 0x10030510, myDirect3DCreate9);
}
