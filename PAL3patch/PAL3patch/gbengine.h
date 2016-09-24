#ifndef PAL3PATCH_GBENGINE_H
#define PAL3PATCH_GBENGINE_H

// types

enum GAME_STATE {
    GAME_NONE,
    GAME_UI,
    GAME_SCENE,
    GAME_COMBAT,
    GAME_SUBGAME_ENCAMPMENT,
    GAME_SUBGAME_SKEE,
    GAME_HIDE_FIGHT_FIXME, // FIXME: unsure
    GAME_SUBGAME_HIDEANDSEEK,
    GAME_SUBGAME_NIGHTADVENTURE,
    GAME_OVER,
};

struct gbGfxDriverInfo {
    int type;
    int subtype;
    int width;
    int height;
    int colorbits;
    int depthbits;
    int stensilbits;
    int nbackbuf;
    int fullscreen;
    int refreshrate;
    char winname[0x80];
    DWORD hinst;
    HWND hgfxwnd;
    int newwindow;
    int waitforverticalblank;
};

struct gbGfxManager_D3D {
    char padding1[0xC];
    struct gbGfxDriverInfo DrvInfo;
    char padding2[0x5A4];
    int m_bShowCursor;
    char padding3[0x70];
    D3DPRESENT_PARAMETERS m_d3dpp;
    struct IDirect3D9 *m_pD3D;
    struct IDirect3DDevice9 *m_pd3dDevice;
    D3DCAPS9 m_d3dCaps;
    D3DSURFACE_DESC m_d3dsdBackBuffer;
    char m_strDeviceStats[0x5A];
    long m_ErrorCode;
    int m_bDeviceLost;
    struct IDirect3DSurface9 *m_pBackBuffer;
    int m_DXTSupported[0x5];
    struct IDirect3DVertexBuffer9 *m_pCacheVB;
    struct IDirect3DIndexBuffer9 *m_pCacheIB;
};

enum gbPixelFmtType { 
    GB_PFT_UNKNOWN,
    GB_PFT_R8G8B8,
    GB_PFT_A8R8G8B8,
    GB_PFT_R5G6B5,
    GB_PFT_A1R5G5B5,
    GB_PFT_A4R4G4B4,
    GB_PFT_P8,
    GB_PFT_A8,
    GB_PFT_X8R8G8B8,
    GB_PFT_X1R5G5B5,
    GB_PFT_X4R4G4B4,
};

struct gbSurfaceDesc {
    int width;
    int height;
    enum gbPixelFmtType format;
    int pitch;
    void *pbits;
};

struct gbTexture_D3D {
    char padding[0x54];
    struct IDirect3DBaseTexture9 *pTex;
    struct IDirect3DSurface9 *pDS;
    ULONG m_ImgFormat;
};
struct RenderTarget {
    int m_iMode;
    struct gbTexture_D3D m_Texture;
    struct gbTexture_D3D m_ScreenPlane;
    int m_nState;
};

struct gbColorQuad {
    union {
        int ColorQuadFmt;
        struct {
            BYTE Blue;
            BYTE Green;
            BYTE Red;
            BYTE Alpha;
        };
        DWORD Color;
    };
};

struct PtrArray {
    void **m_pData;
    int m_nSize;
    int m_nMaxSize;
    int m_nGrowBy;
};

struct UIWnd {
    void **vfptr;
    struct gbColorQuad m_fontcolor;
    struct gbColorQuad m_wndcolor;
    DWORD m_id;
    RECT m_rect;
    struct UIWnd *m_pfather;
    struct PtrArray m_childs;
    int m_bcreateok;
    int m_bvisible;
    int m_benable;
    int m_bfocus;
};

struct gbUIQuad {
    float sx;
    float sy;
    float ex;
    float ey;
    float su;
    float sv;
    float eu;
    float ev;
    float z;
    struct gbColorQuad color;
};

struct CTrail {
    float m_fTime;
    BYTE m_bSupport;
    BYTE m_bEnable;
    struct gbCamera *m_pCam;
    struct gbUIQuad m_ScreenQuad;
    ULONG m_dwRenderCounter;
    ULONG m_dwRT;
    struct gbTexture_D3D m_texRT[0x8];
    struct gbRenderEffect *m_eft;
    struct IDirect3DSurface9 *m_OriginSurface;
};


struct gbPixelFormat {
    enum gbPixelFmtType Type;
    int Bpp;
    struct gbColorQuad *Pal;
    unsigned int r_mask;
    unsigned int g_mask;
    unsigned int b_mask;
    unsigned int a_mask;
    unsigned int r_shift;
    unsigned int g_shift;
    unsigned int b_shift;
    unsigned int a_shift;
};
struct gbImage2DInfo {
    int isdiskinfo;
    int goal;
    int ngrade;
    int detail[8];
    int wdiv[8];
    int hdiv[8];
    int wrap_u;
    int wrap_v;
    int mipmap;
    int minfilter;
    int magfilter;
};
struct gbImage2D {
    struct gbImage2DInfo ImgInfo;
    int Width;
    int Height;
    struct gbPixelFormat PixelFmt;
    int BitCount;
    char *pBits;
    struct gbColorQuad *pPalette;
    int numPalEntry;
    struct gbColorQuad ColorKey;
};



struct gbVFile;
struct _iobuf;
enum ECPKMode { 
    CPKM_Null,
    CPKM_Normal,
    CPKM_FileMapping,
    CPKM_Overlapped,
    CPKM_End,
};
struct CPKHeader {
    ULONG dwLable;
    ULONG dwVersion;
    ULONG dwTableStart;
    ULONG dwDataStart;
    ULONG dwMaxFileNum;
    ULONG dwFileNum;
    ULONG dwIsFormatted;
    ULONG dwSizeOfHeader;
    ULONG dwValidTableNum;
    ULONG dwMaxTableNum;
    ULONG dwFragmentNum;
    ULONG dwPackageSize;
    ULONG dwReserved[0x14];
};
struct CPKTable {
    ULONG dwCRC;
    ULONG dwFlag;
    ULONG dwFatherCRC;
    ULONG dwStartPos;
    ULONG dwPackedSize;
    ULONG dwOriginSize;
    ULONG dwExtraInfoSize;
};
struct CPKFile {
    BYTE bValid;
    ULONG dwCRC;
    ULONG dwFatherCRC;
    int nTableIndex;
    void *lpMapAddress;
    void *lpStartAddress;
    ULONG dwOffset;
    BYTE bCompressed;
    void *lpMem;
    ULONG dwFileSize;
    ULONG dwPointer;
    struct CPKTable *pTableItem;
};
struct CPK {
    ULONG m_dwAllocGranularity;
    enum ECPKMode m_eMode;
    struct CPKHeader m_CPKHeader;
    struct CPKTable m_CPKTable[0x8000];
    struct gbVFile *m_pgbVFile[0x8];
    BYTE m_bLoaded;
    ULONG m_dwCPKHandle;
    ULONG m_dwCPKMappingHandle;
    char m_szCPKFileName[0x104];
    int m_nOpenedFileNum;
};
struct gbVFileSystem {
    int IsInit;
    char* rtDirectory;
    struct gbVFile *pFileBuffer;
    int Type;
    struct _iobuf *m_pckfp;
    DWORD m_itemcount;
    struct CPK m_cpk;
};






struct gbDynVertBuf;
struct gbRenderEffect;
struct gbTextureArray;
struct gbPrintFont;







// D3DX types and functions
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

#define D3DXCreateTextureFromFileInMemoryEx ((HRESULT WINAPI (*)(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture)) (gboffset + 0x10034211))
#define D3DXGetImageInfoFromFileInMemory ((HRESULT WINAPI (*)(LPCVOID pSrcData, UINT SrcDataSize, D3DXIMAGE_INFO *pSrcInfo)) (gboffset + 0x10032E69))

// D3DX internels (summer 2003 SDK)
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






// functions
#define gbGfxManager_D3D_Reset3DEnvironment(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001AC50, int, struct gbGfxManager_D3D *), this)
extern enum gbPixelFmtType gbGfxManager_D3D_GetBackBufferFormat(struct gbGfxManager_D3D *this);
extern void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus);
#define UIWnd_SetRect(this, rect) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00445FA0, void, struct UIWnd *, RECT *), this, rect)
#define gbTexture_D3D_CreateForRenderTarget(this, width, height, format) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001BF80, int, struct gbTexture_D3D *, int, int, enum gbPixelFmtType), this, width, height, format)
#define gbTexture_D3D_CreateFromFileMemory(this, data, len) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001C010, void, struct gbTexture_D3D *, void *, int), this, data, len)
#define gbmalloc ((malloc_funcptr_t) (gboffset + 0x100E4B0D))
#define gbfree ((free_funcptr_t) (gboffset + 0x100E4B99))

// PAL3 functions
#define PrepareDir ((int (*)(void)) TOPTR(0x00538320))


// global variables
#define game_state (*(int *) TOPTR(0x00BFDA6C))
#define is_window_active (*(int *) TOPTR(0x005833B8))
#define g_GfxMgr (*(struct gbGfxManager_D3D **) 0x00BFDA60)
#define g_pVFileSys (*(struct gbVFileSystem **) (gboffset + 0x1015D3A8))


#endif
