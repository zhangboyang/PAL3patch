#ifndef PAL3APATCH_D3DX9WRAPPER_H
#define PAL3APATCH_D3DX9WRAPPER_H
// PATCHAPI DEFINITIONS

extern PATCHAPI HMODULE d3dx9_43;


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

extern HRESULT (WINAPI *myD3DXCreateFontW)(LPDIRECT3DDEVICE9, UINT, UINT, UINT, UINT, BOOL, DWORD, DWORD, DWORD, DWORD, LPCWSTR, LPD3DXFONT *);
extern HRESULT (WINAPI *myD3DXCreateSprite)(LPDIRECT3DDEVICE9, LPD3DXSPRITE *);
extern HRESULT (WINAPI *myD3DXGetImageInfoFromFileInMemory)(LPCVOID, UINT, D3DXIMAGE_INFO *);
extern HRESULT (WINAPI *myD3DXLoadSurfaceFromFileInMemory)(LPDIRECT3DSURFACE9, CONST PALETTEENTRY *, CONST RECT *, LPCVOID, UINT, CONST RECT *, DWORD, D3DCOLOR, D3DXIMAGE_INFO *);
extern HRESULT (WINAPI *myD3DXFillTexture)(LPDIRECT3DTEXTURE9, LPD3DXFILL2D, LPVOID);
extern HRESULT (WINAPI *myD3DXSaveSurfaceToFileA)(LPCTSTR, D3DXIMAGE_FILEFORMAT, LPDIRECT3DSURFACE9, const PALETTEENTRY *, const RECT *);

extern unsigned gbD3DXCreateEffect;

extern void fuse_d3dx9_wrapper(void);
extern void init_d3dx9_wrapper(void);

// no ID3DXSprite vftable changes between d3dx9_21 and d3dx9_43
#define myID3DXSprite_Release(p)        ID3DXSprite_Release(p)
#define myID3DXSprite_Draw(p,a,b,c,d,e) ID3DXSprite_Draw(p,a,b,c,d,e)
#define myID3DXSprite_Begin(p,a)        ID3DXSprite_Begin(p,a)
#define myID3DXSprite_End(p)            ID3DXSprite_End(p)
#define myID3DXSprite_OnLostDevice(p)   ID3DXSprite_OnLostDevice(p)
#define myID3DXSprite_OnResetDevice(p)  ID3DXSprite_OnResetDevice(p)

#define myID3DXFont_Release(p)               (d3dx9_43 ? ID3DXFont_Release(((ID3DXFont43 *)p)) : ID3DXFont_Release(p))
#define myID3DXFont_DrawTextW(p,a,b,c,d,e,f) (d3dx9_43 ? ID3DXFont_DrawTextW(((ID3DXFont43 *)p),a,b,c,d,e,f) : ID3DXFont_DrawTextW(p,a,b,c,d,e,f))
#define myID3DXFont_PreloadCharacters(p,a,b) (d3dx9_43 ? ID3DXFont_PreloadCharacters(((ID3DXFont43 *)p),a,b) : ID3DXFont_PreloadCharacters(p,a,b))
#define myID3DXFont_PreloadTextW(p,a,b)      (d3dx9_43 ? ID3DXFont_PreloadTextW(((ID3DXFont43 *)p),a,b) : ID3DXFont_PreloadTextW(p,a,b))
#define myID3DXFont_OnLostDevice(p)          (d3dx9_43 ? ID3DXFont_OnLostDevice(((ID3DXFont43 *)p)) : ID3DXFont_OnLostDevice(p))
#define myID3DXFont_OnResetDevice(p)         (d3dx9_43 ? ID3DXFont_OnResetDevice(((ID3DXFont43 *)p)) : ID3DXFont_OnResetDevice(p))

#ifdef USE_D3DX9D
#define D3DX9_43_DLL "d3dx9d_43.dll"
#else
#define D3DX9_43_DLL "d3dx9_43.dll"
#endif
#define D3DCOMPILER_43_DLL "d3dcompiler_43.dll"

#endif
#endif
