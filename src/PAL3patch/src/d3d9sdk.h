#ifndef PAL3PATCH_D3D9SDK_H
#define PAL3PATCH_D3D9SDK_H

// DirectInput

#define DIRECTINPUT_VERSION 0x0800

#ifdef HAVE_D3D9SDK
#include <dinput.h>
#else

struct IDirectInput8A;
struct IDirectInputDevice8A;
typedef struct IDirectInput8A IDirectInput8A;
typedef struct IDirectInputDevice8A IDirectInputDevice8A;
#define IDirectInput8 IDirectInput8A
#define IDirectInputDevice8 IDirectInputDevice8A

typedef struct _DIMOUSESTATE2 {
    LONG    lX;
    LONG    lY;
    LONG    lZ;
    BYTE    rgbButtons[8];
} DIMOUSESTATE2, *LPDIMOUSESTATE2;

#endif




// D3DX


#if defined(_MSC_VER) && !defined(DYNLINK_D3DX9_AT_RUNTIME)
#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif
#if _MSC_VER >= 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif
#endif


#ifdef HAVE_D3D9SDK
// include d3dx9.h only, do not include d3d9.h here
// d3dx9.h will include d3d9.h automaticly
// this avoid confilits between DirectX SDK headers and DirectX headers in Windows 8 SDK
#include <d3dx9.h>

#else /* HAVE_D3D9SDK */

#include <d3d9.h>

// make gcc happy, not generate error for typedef redefinition
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC system_header
#endif

#define D3DX_SDK_VERSION 21

// D3DX9 types and functions
// this is copied from Microsoft D3DX headers

typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0,
    D3DXIFF_JPG = 1,
    D3DXIFF_TGA = 2,
    D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4,
    D3DXIFF_PPM = 5,
    D3DXIFF_DIB = 6,
    D3DXIFF_HDR = 7,
    D3DXIFF_PFM = 8,
    D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;
typedef struct _D3DXIMAGE_INFO {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
    D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

#define D3DX_DEFAULT ((UINT) -1)

typedef struct _D3DMATRIX D3DXMATRIX, *LPD3DXMATRIX;
typedef struct D3DXVECTOR2 {
    FLOAT x, y;
} D3DXVECTOR2, *LPD3DXVECTOR2;
typedef struct _D3DVECTOR D3DXVECTOR3, *LPD3DXVECTOR3;
typedef struct D3DXVECTOR4 {
    FLOAT x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;

//////////////////////////////////////////////////////////////////////////////
// D3DXSPRITE flags:
// -----------------
// D3DXSPRITE_DONOTSAVESTATE
//   Specifies device state is not to be saved and restored in Begin/End.
// D3DXSPRITE_DONOTMODIFY_RENDERSTATE
//   Specifies device render state is not to be changed in Begin.  The device
//   is assumed to be in a valid state to draw vertices containing POSITION0, 
//   TEXCOORD0, and COLOR0 data.
// D3DXSPRITE_OBJECTSPACE
//   The WORLD, VIEW, and PROJECTION transforms are NOT modified.  The 
//   transforms currently set to the device are used to transform the sprites 
//   when the batch is drawn (at Flush or End).  If this is not specified, 
//   WORLD, VIEW, and PROJECTION transforms are modified so that sprites are 
//   drawn in screenspace coordinates.
// D3DXSPRITE_BILLBOARD
//   Rotates each sprite about its center so that it is facing the viewer.
// D3DXSPRITE_ALPHABLEND
//   Enables ALPHABLEND(SRCALPHA, INVSRCALPHA) and ALPHATEST(alpha > 0).
//   ID3DXFont expects this to be set when drawing text.
// D3DXSPRITE_SORT_TEXTURE
//   Sprites are sorted by texture prior to drawing.  This is recommended when
//   drawing non-overlapping sprites of uniform depth.  For example, drawing
//   screen-aligned text with ID3DXFont.
// D3DXSPRITE_SORT_DEPTH_FRONTTOBACK
//   Sprites are sorted by depth front-to-back prior to drawing.  This is 
//   recommended when drawing opaque sprites of varying depths.
// D3DXSPRITE_SORT_DEPTH_BACKTOFRONT
//   Sprites are sorted by depth back-to-front prior to drawing.  This is 
//   recommended when drawing transparent sprites of varying depths.
//////////////////////////////////////////////////////////////////////////////

#define D3DXSPRITE_DONOTSAVESTATE               (1 << 0)
#define D3DXSPRITE_DONOTMODIFY_RENDERSTATE      (1 << 1)
#define D3DXSPRITE_OBJECTSPACE                  (1 << 2)
#define D3DXSPRITE_BILLBOARD                    (1 << 3)
#define D3DXSPRITE_ALPHABLEND                   (1 << 4)
#define D3DXSPRITE_SORT_TEXTURE                 (1 << 5)
#define D3DXSPRITE_SORT_DEPTH_FRONTTOBACK       (1 << 6)
#define D3DXSPRITE_SORT_DEPTH_BACKTOFRONT       (1 << 7)

//////////////////////////////////////////////////////////////////////////////
// ID3DXSprite:
// ------------
// This object intends to provide an easy way to drawing sprites using D3D.
//
// Begin - 
//    Prepares device for drawing sprites.
//
// Draw -
//    Draws a sprite.  Before transformation, the sprite is the size of 
//    SrcRect, with its top-left corner specified by Position.  The color 
//    and alpha channels are modulated by Color.
//
// Flush -
//    Forces all batched sprites to submitted to the device.
//
// End - 
//    Restores device state to how it was when Begin was called.
//
// OnLostDevice, OnResetDevice -
//    Call OnLostDevice() on this object before calling Reset() on the
//    device, so that this object can release any stateblocks and video
//    memory resources.  After Reset(), the call OnResetDevice().
//////////////////////////////////////////////////////////////////////////////

typedef interface ID3DXSprite ID3DXSprite;
typedef interface ID3DXSprite *LPD3DXSPRITE;


// {D4715B38-6C44-472a-9024-6E2B0321CAC6}
DEFINE_GUID( IID_ID3DXSprite, 
0xd4715b38, 0x6c44, 0x472a, 0x90, 0x24, 0x6e, 0x2b, 0x3, 0x21, 0xca, 0xc6);


#undef INTERFACE
#define INTERFACE ID3DXSprite

DECLARE_INTERFACE_(ID3DXSprite, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXSprite
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;

    STDMETHOD(GetTransform)(THIS_ D3DXMATRIX *pTransform) PURE;
    STDMETHOD(SetTransform)(THIS_ CONST D3DXMATRIX *pTransform) PURE;

    STDMETHOD(SetWorldViewRH)(THIS_ CONST D3DXMATRIX *pWorld, CONST D3DXMATRIX *pView) PURE;
    STDMETHOD(SetWorldViewLH)(THIS_ CONST D3DXMATRIX *pWorld, CONST D3DXMATRIX *pView) PURE;

    STDMETHOD(Begin)(THIS_ DWORD Flags) PURE;
    STDMETHOD(Draw)(THIS_ LPDIRECT3DTEXTURE9 pTexture, CONST RECT *pSrcRect, CONST D3DXVECTOR3 *pCenter, CONST D3DXVECTOR3 *pPosition, D3DCOLOR Color) PURE;
    STDMETHOD(Flush)(THIS) PURE;
    STDMETHOD(End)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// ID3DXFont:
// ----------
// Font objects contain the textures and resources needed to render a specific 
// font on a specific device.
//
// GetGlyphData -
//    Returns glyph cache data, for a given glyph.
//
// PreloadCharacters/PreloadGlyphs/PreloadText -
//    Preloads glyphs into the glyph cache textures.
//
// DrawText -
//    Draws formatted text on a D3D device.  Some parameters are 
//    surprisingly similar to those of GDI's DrawText function.  See GDI 
//    documentation for a detailed description of these parameters.
//    If pSprite is NULL, an internal sprite object will be used.
//
// OnLostDevice, OnResetDevice -
//    Call OnLostDevice() on this object before calling Reset() on the
//    device, so that this object can release any stateblocks and video
//    memory resources.  After Reset(), the call OnResetDevice().
//////////////////////////////////////////////////////////////////////////////

typedef struct _D3DXFONT_DESCA
{
    UINT Height;
    UINT Width;
    UINT Weight;
    UINT MipLevels;
    BOOL Italic;
    BYTE CharSet;
    BYTE OutputPrecision;
    BYTE Quality;
    BYTE PitchAndFamily;
    CHAR FaceName[LF_FACESIZE];

} D3DXFONT_DESCA, *LPD3DXFONT_DESCA;

typedef struct _D3DXFONT_DESCW
{
    UINT Height;
    UINT Width;
    UINT Weight;
    UINT MipLevels;
    BOOL Italic;
    BYTE CharSet;
    BYTE OutputPrecision;
    BYTE Quality;
    BYTE PitchAndFamily;
    WCHAR FaceName[LF_FACESIZE];

} D3DXFONT_DESCW, *LPD3DXFONT_DESCW;

typedef D3DXFONT_DESCW D3DXFONT_DESC;
typedef LPD3DXFONT_DESCW LPD3DXFONT_DESC;

typedef interface ID3DXFont ID3DXFont;
typedef interface ID3DXFont *LPD3DXFONT;

// {0B8D1536-9EEC-49b0-A5AD-93CF63AFB7C6}
DEFINE_GUID( IID_ID3DXFont, 
0xb8d1536, 0x9eec, 0x49b0, 0xa5, 0xad, 0x93, 0xcf, 0x63, 0xaf, 0xb7, 0xc6);

#undef INTERFACE
#define INTERFACE ID3DXFont
DECLARE_INTERFACE_(ID3DXFont, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXFont
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9 *ppDevice) PURE;
    STDMETHOD(GetDescA)(THIS_ D3DXFONT_DESCA *pDesc) PURE;
    STDMETHOD(GetDescW)(THIS_ D3DXFONT_DESCW *pDesc) PURE;

    STDMETHOD_(HDC, GetDC)(THIS) PURE;
    STDMETHOD(GetGlyphData)(THIS_ UINT Glyph, LPDIRECT3DTEXTURE9 *ppTexture, RECT *pBlackBox, POINT *pCellInc) PURE;

    STDMETHOD(PreloadCharacters)(THIS_ UINT First, UINT Last) PURE;
    STDMETHOD(PreloadGlyphs)(THIS_ UINT First, UINT Last) PURE;
    STDMETHOD(PreloadTextA)(THIS_ LPCSTR pString, INT Count) PURE;
    STDMETHOD(PreloadTextW)(THIS_ LPCWSTR pString, INT Count) PURE;

    STDMETHOD_(INT, DrawTextA)(THIS_ LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;
    STDMETHOD_(INT, DrawTextW)(THIS_ LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;

};



typedef VOID (WINAPI *LPD3DXFILL2D)(D3DXVECTOR4 *pOut, 
    CONST D3DXVECTOR2 *pTexCoord, CONST D3DXVECTOR2 *pTexelSize, LPVOID pData);


#endif /* HAVE_D3D9SDK */



#ifdef DYNLINK_D3DX9_AT_RUNTIME
#define DECL_D3DX9FUNC(func) (WINAPI *CONCAT(p, func))
#define D3DXFUNC(func) CONCAT(p, func)
extern void d3dx9_dynlink();
#else
#define DECL_D3DX9FUNC(func) (WINAPI func)
#define D3DXFUNC(func) func
#endif



#if defined(DYNLINK_D3DX9_AT_RUNTIME) || !defined(HAVE_D3D9SDK)

// functions for D3DX9
HRESULT DECL_D3DX9FUNC(D3DXCreateFontW)(
    LPDIRECT3DDEVICE9       pDevice,  
    UINT                    Height,
    UINT                    Width,
    UINT                    Weight,
    UINT                    MipLevels,
    BOOL                    Italic,
    DWORD                   CharSet,
    DWORD                   OutputPrecision,
    DWORD                   Quality,
    DWORD                   PitchAndFamily,
    LPCWSTR                 pFaceName,
    LPD3DXFONT*             ppFont);
HRESULT DECL_D3DX9FUNC(D3DXCreateSprite)( 
    LPDIRECT3DDEVICE9   pDevice, 
    LPD3DXSPRITE*       ppSprite);


HRESULT DECL_D3DX9FUNC(D3DXGetImageInfoFromFileInMemory)(
    LPCVOID pSrcData,
    UINT SrcDataSize,
    D3DXIMAGE_INFO *pSrcInfo
);
HRESULT DECL_D3DX9FUNC(D3DXLoadSurfaceFromFileInMemory)(
    LPDIRECT3DSURFACE9 pDestSurface,
    CONST PALETTEENTRY *pDestPalette,
    CONST RECT *pDestRect,
    LPCVOID pSrcData,
    UINT SrcData,
    CONST RECT *pSrcRect,
    DWORD Filter,
    D3DCOLOR ColorKey,
    D3DXIMAGE_INFO *pSrcInfo
);
HRESULT DECL_D3DX9FUNC(D3DXFillTexture)(
    LPDIRECT3DTEXTURE9 pTexture,
    LPD3DXFILL2D pFunction,
    LPVOID pData
);
HRESULT DECL_D3DX9FUNC(D3DXSaveSurfaceToFileA)(
    LPCTSTR pDestFile,
    D3DXIMAGE_FILEFORMAT DestFormat,
    LPDIRECT3DSURFACE9 pSrcSurface,
    const PALETTEENTRY *pSrcPalette,
    const RECT *pSrcRect
);

#endif

// method macros

#define ID3DXSprite_Release(p) (p)->lpVtbl->Release(p)
#define ID3DXSprite_SetTransform(p,a) (p)->lpVtbl->SetTransform(p,a)
#define ID3DXSprite_SetWorldViewRH(p,a,b) (p)->lpVtbl->SetWorldViewRH(p,a,b)
#define ID3DXSprite_SetWorldViewLH(p,a,b) (p)->lpVtbl->SetWorldViewLH(p,a,b)
#define ID3DXSprite_Begin(p,a) (p)->lpVtbl->Begin(p,a)
#define ID3DXSprite_End(p) (p)->lpVtbl->End(p)
#define ID3DXSprite_OnLostDevice(p) (p)->lpVtbl->OnLostDevice(p)
#define ID3DXSprite_OnResetDevice(p) (p)->lpVtbl->OnResetDevice(p)

#define ID3DXFont_Release(p) (p)->lpVtbl->Release(p)
#define ID3DXFont_DrawTextA(p,a,b,c,d,e,f) (p)->lpVtbl->DrawTextA(p,a,b,c,d,e,f)
#define ID3DXFont_DrawTextW(p,a,b,c,d,e,f) (p)->lpVtbl->DrawTextW(p,a,b,c,d,e,f)
#define ID3DXFont_PreloadCharacters(p,a,b) (p)->lpVtbl->PreloadCharacters(p,a,b)
#define ID3DXFont_PreloadTextW(p,a,b) (p)->lpVtbl->PreloadTextW(p,a,b)
#define ID3DXFont_OnLostDevice(p) (p)->lpVtbl->OnLostDevice(p)
#define ID3DXFont_OnResetDevice(p) (p)->lpVtbl->OnResetDevice(p)


#endif
