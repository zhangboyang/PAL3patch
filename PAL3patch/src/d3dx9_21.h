#ifndef PAL3PATCH_D3DX9_H
#define PAL3PATCH_D3DX9_H

// make gcc happy, not generate error for typedef redefinition
#if __GNUC__ >=3
#pragma GCC system_header
#endif

/*
    PAL3's gbengine.dll uses D3DX9 from summer 2003 SDK
    the D3DX_SDK_VERSION is 21 in this SDK, so the DLL name is d3dx9_21.dll
    in fact, Microsoft has never release such DLL, this DLL is compiled by myself
*/
#define D3DX9_DLL "d3dx9_21.dll"



// D3DX9 types and functions
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
typedef struct _D3DVECTOR D3DXVECTOR3, *LPD3DXVECTOR3;


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

#define ID3DXSprite_Release(p) (p)->lpVtbl->Release(p)
#define ID3DXSprite_SetTransform(p,a) (p)->lpVtbl->SetTransform(p,a)
#define ID3DXSprite_SetWorldViewRH(p,a,b) (p)->lpVtbl->SetWorldViewRH(p,a,b)
#define ID3DXSprite_SetWorldViewLH(p,a,b) (p)->lpVtbl->SetWorldViewLH(p,a,b)
#define ID3DXSprite_Begin(p,a) (p)->lpVtbl->Begin(p,a)
#define ID3DXSprite_End(p) (p)->lpVtbl->End(p)
#define ID3DXSprite_OnLostDevice(p) (p)->lpVtbl->OnLostDevice(p)
#define ID3DXSprite_OnResetDevice(p) (p)->lpVtbl->OnResetDevice(p)

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

#define ID3DXFont_Release(p) (p)->lpVtbl->Release(p)
#define ID3DXFont_DrawTextA(p,a,b,c,d,e,f) (p)->lpVtbl->DrawTextA(p,a,b,c,d,e,f)
#define ID3DXFont_DrawTextW(p,a,b,c,d,e,f) (p)->lpVtbl->DrawTextW(p,a,b,c,d,e,f)
#define ID3DXFont_PreloadCharacters(p,a,b) (p)->lpVtbl->PreloadCharacters(p,a,b)
#define ID3DXFont_PreloadTextW(p,a,b) (p)->lpVtbl->PreloadTextW(p,a,b)
#define ID3DXFont_OnLostDevice(p) (p)->lpVtbl->OnLostDevice(p)
#define ID3DXFont_OnResetDevice(p) (p)->lpVtbl->OnResetDevice(p)








// D3DX CImage internels (summer 2003 SDK)
struct D3DXTex_CImage {
    D3DFORMAT Format;
    void *pBits;
    DWORD *pPalette; // A8B8G8R8, byte0 = B, byte1 = G, byte2 = R, byte3 = A
    UINT Width;
    UINT Height;
    UINT Depth;
    DWORD gap18[6];
    INT Pitch;
    DWORD gap34[1];
    INT pBits_Valid;
    INT pPalette_Valid;
    INT Valid;
    D3DRESOURCETYPE ResourceType;
    D3DXIMAGE_FILEFORMAT ImageFileFormat;
    struct D3DXTex_CImage *pNext1;
    struct D3DXTex_CImage *pNext2;
};
#define D3DXTex_CImage_ctor(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x100346CA, void, struct D3DXTex_CImage *), this)
#define D3DXTex_CImage_dtor(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x100346E3, void, struct D3DXTex_CImage *), this)
#define D3DXTex_CImage_Load(this, data, len, pinfo, flag) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10037159, int, struct D3DXTex_CImage *, const void *, unsigned, D3DXIMAGE_INFO *, int), this, data, len, pinfo, flag)


#ifndef USE_MSVC_LINKER
#define PAL3PATCH_D3DX9FUNC(func) (WINAPI *func)
#else
#define PAL3PATCH_D3DX9FUNC(func) (WINAPI func)
#endif

// function pointers for D3DX9
extern HRESULT PAL3PATCH_D3DX9FUNC(D3DXSaveTextureToFileA)(LPCTSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE9 pSrcTexture, const PALETTEENTRY *pSrcPalette);
extern HRESULT PAL3PATCH_D3DX9FUNC(D3DXCreateFontW)(
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
extern HRESULT PAL3PATCH_D3DX9FUNC(D3DXCreateSprite)( 
    LPDIRECT3DDEVICE9   pDevice, 
    LPD3DXSPRITE*       ppSprite);

extern void d3dx9_dynlink();

#endif
