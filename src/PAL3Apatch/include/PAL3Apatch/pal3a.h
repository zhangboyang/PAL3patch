#ifndef PAL3APATCH_PAL3A_H
#define PAL3APATCH_PAL3A_H
// PATCHAPI DEFINITIONS


extern PATCHAPI void *load_image_bits(void *filedata, unsigned filelen, int *width, int *height, int *bitcount, const struct memory_allocator *mem_allocator);
extern PATCHAPI void ensure_cooperative_level(int requirefocus);
extern PATCHAPI void clamp_rect(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom);
extern PATCHAPI void copy_bits(void *dst, int dst_pitch, int dst_x, int dst_y, void *src, int src_pitch, int src_x, int src_y, int width, int height, int bitcount);
extern PATCHAPI void fill_texture(IDirect3DTexture9 *tex, D3DCOLOR color);
extern PATCHAPI void set_d3dxfont_matrices(IDirect3DDevice9 *pd3dDevice);


#ifdef USE_PAL3A_DEFINITIONS


struct gbAudioMgrDesc {
    unsigned int frequence;
    unsigned int bits;
    unsigned int channels;
    char provider[256];
};

struct PtrArray {
    void **m_pData;
    int m_nSize;
    int m_nMaxSize;
    int m_nGrowBy;
};

struct gbColorQuad {
    union {
        struct {
            unsigned char Blue;
            unsigned char Green;
            unsigned char Red;
            unsigned char Alpha;
        };
        unsigned int Color;
    };
};

struct UIWnd {
    struct UIWndVtbl *vfptr;
    struct gbColorQuad m_fontcolor;
    struct gbColorQuad m_wndcolor;
    unsigned int m_id;
    RECT m_rect;
    struct UIWnd *m_pfather;
    struct PtrArray m_childs;
    int m_bcreateok;
    int m_bvisible;
    int m_benable;
    int m_bfocus;
};

struct gbMatrix4 {
    float m[4][4];
};

struct gbVec3D {
    float x;
    float y;
    float z;
};

struct gbColorFlt {
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct gbViewPort {
    int x;
    int y;
    int width;
    int height;
    float minz;
    float maxz;
};

struct gbClearParam {
    struct gbColorQuad color;
    float depth;
    int stencil;
    unsigned int clearflag;
};

struct gbGfxStatistic {
    int nRFaces;
    int nRVerts;
    int nStaticVerts;
    int nStaticIndex;
    int nSrcVerts;
    int nSrcIndex;
    int nTexSwitch;
    float FPS;
    double TimeAccum;
    unsigned int Frame;
    unsigned int PrevFrame;
};

enum gbLight_gbLightType {
    GBLIGHT_POINT = 0x1,
    GBLIGHT_SPOT = 0x2,
    GBLIGHT_DIRECT = 0x3,
};

struct gbLight {
    enum gbLight_gbLightType type;
    struct gbColorFlt diffuse;
    struct gbColorFlt specular;
    struct gbColorFlt ambient;
    struct gbVec3D position;
    struct gbVec3D direction;
    float range;
    float falloff;
    float attenuation0;
    float attenuation1;
    float attenuation2;
    float theta;
    float phi;
};

enum gbFogParam_gbFogType {
    FOGTYPE_NULL = 0x0,
    FOGTYPE_DIST = 0x1,
    FOGTYPE_PLANE = 0x2,
    FOGTYPE_SPHERE = 0x3,
};

enum gbFogParam_gbFogMode {
    FOGMODE_LINEAR = 0x0,
    FOGMODE_EXP = 0x1,
    FOGMODE_EXP2 = 0x2,
};

struct gbFogParam {
    enum gbFogParam_gbFogType type;
    struct gbColorFlt color;
    enum gbFogParam_gbFogMode mode;
    float start;
    float end;
    float density;
    float plane[4];
};

enum VertexProcessingType {
    SOFTWARE_VP = 0x0,
    MIXED_VP = 0x1,
    HARDWARE_VP = 0x2,
    PURE_HARDWARE_VP = 0x3,
};

struct CD3DEnumeration {
    struct IDirect3D9 *m_pD3D;
    struct CArrayList *m_pAdapterInfoList;
    char (__cdecl *ConfirmDeviceCallback)(D3DCAPS9 *, enum VertexProcessingType, D3DFORMAT, D3DFORMAT);
    unsigned int AppMinFullscreenWidth;
    unsigned int AppMinFullscreenHeight;
    unsigned int AppMinColorChannelBits;
    unsigned int AppMinAlphaChannelBits;
    unsigned int AppMinDepthBits;
    unsigned int AppMinStencilBits;
    char AppUsesDepthBuffer;
    char AppUsesMixedVP;
    char AppRequiresWindowed;
    char AppRequiresFullscreen;
    struct CArrayList *m_pAllowedAdapterFormatList;
};

struct D3DDriverBug {
    int Gamma_LowByte;
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
    char winname[128];
    DWORD hinst;
    DWORD hgfxwnd;
    int newwindow;
    int waitforverticalblank;
};

struct gbDynArray_pgbResource {
    struct gbResource **pBuffer;
    int MaxNum;
    int CurNum;
};

struct gbResManager {
    struct gbDynArray_pgbResource AllResPt;
};

struct CD3DSettings {
    char IsWindowed;
    struct D3DAdapterInfo *pWindowed_AdapterInfo;
    struct D3DDeviceInfo *pWindowed_DeviceInfo;
    struct D3DDeviceCombo *pWindowed_DeviceCombo;
    D3DDISPLAYMODE Windowed_DisplayMode;
    D3DFORMAT Windowed_DepthStencilBufferFormat;
    D3DMULTISAMPLE_TYPE Windowed_MultisampleType;
    unsigned int Windowed_MultisampleQuality;
    enum VertexProcessingType Windowed_VertexProcessingType;
    unsigned int Windowed_PresentInterval;
    int Windowed_Width;
    int Windowed_Height;
    struct D3DAdapterInfo *pFullscreen_AdapterInfo;
    struct D3DDeviceInfo *pFullscreen_DeviceInfo;
    struct D3DDeviceCombo *pFullscreen_DeviceCombo;
    D3DDISPLAYMODE Fullscreen_DisplayMode;
    D3DFORMAT Fullscreen_DepthStencilBufferFormat;
    D3DMULTISAMPLE_TYPE Fullscreen_MultisampleType;
    unsigned int Fullscreen_MultisampleQuality;
    enum VertexProcessingType Fullscreen_VertexProcessingType;
    unsigned int Fullscreen_PresentInterval;
};

struct gbGfxManager {
    struct gbGfxManagerVtbl *vfptr;
    DWORD gap4[1];
    unsigned int hGfxWnd;
    struct gbGfxDriverInfo DrvInfo;
    struct gbVertPoolMgr *pVertPoolMgr;
    struct gbEffectMgr *pEffectMgr;
    struct gbResManager *pTexResMgr;
    struct gbPrintFontMgr *pFontMgr;
    struct gbMatrixStack *pMatrixStack[4];
    struct gbMatrix4 ViewMatrix;
    struct gbMatrix4 InvViewMatrix;
    struct gbMatrix4 WorldMatrix;
    int CurZBias;
    int bWireMode;
    struct gbColorFlt AmbientLight;
    float fSelfIllum;
    struct gbSceneGraph *pSceneGraph;
    struct gbViewPort Viewport;
    struct gbClearParam ClearParam;
    double LastTime;
    double CurTime;
    unsigned int LastTick;
    unsigned int CurTick;
    struct gbGfxStatistic Stat;
    struct gbLight AllLight[8];
    int LightEnable[8];
    struct gbFogParam FogParam;
    struct gbTexture *m_pCurTexture[8];
};

struct gbGfxManager_D3D {
    struct gbGfxManager;
    struct CD3DEnumeration m_d3dEnumeration;
    int m_bWindowed;
    RECT m_rcWindowClient;
    RECT m_rcWindowBounds;
    unsigned int m_dwWindowStyle;
    HWND *m_hWnd;
    bool m_bMinimized;
    bool m_bMaximized;
    bool m_bClipCursorWhenFullscreen;
    bool m_bShowCursorWhenFullscreen;
    bool m_bActive;
    unsigned int m_dwCreateFlags;
    struct D3DDriverBug m_DriverBug;
    struct gbResManager m_CursorMgr;
    struct gbCursorRes *m_pActiveCursor;
    int m_bShowCursor;
    int m_bSoftVPMode;
    struct CD3DSettings m_d3dSettings;
    D3DPRESENT_PARAMETERS m_d3dpp;
    IDirect3D9 *m_pD3D;
    IDirect3DDevice9 *m_pd3dDevice;
    D3DCAPS9 m_d3dCaps;
    D3DSURFACE_DESC m_d3dsdBackBuffer;
    char m_strDeviceStats[90];
    int m_ErrorCode;
    int m_bDeviceLost;
    IDirect3DSurface9 *m_pBackBuffer;
    int m_DXTSupported[5];
    IDirect3DVertexBuffer9 *m_pCacheVB;
    IDirect3DIndexBuffer9 *m_pCacheIB;
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
    DWORD color;
};


struct CPKHeader {
    unsigned int dwLable;
    unsigned int dwVersion;
    unsigned int dwTableStart;
    unsigned int dwDataStart;
    unsigned int dwMaxFileNum;
    unsigned int dwFileNum;
    unsigned int dwIsFormatted;
    unsigned int dwSizeOfHeader;
    unsigned int dwValidTableNum;
    unsigned int dwMaxTableNum;
    unsigned int dwFragmentNum;
    unsigned int dwPackageSize;
    unsigned int dwReserved[20];
};

struct CPKTable {
    unsigned int dwCRC;
    unsigned int dwFlag;
    unsigned int dwFatherCRC;
    unsigned int dwStartPos;
    unsigned int dwPackedSize;
    unsigned int dwOriginSize;
    unsigned int dwExtraInfoSize;
};

// GBENGINE functions
#define gbx2x(gbx) (((gbx) + 1.0) * PAL3_s_drvinfo.width / 2.0)
#define gby2y(gby) ((1.0 - (gby)) * PAL3_s_drvinfo.height / 2.0)
#define x2gbx(x) ((x) * 2.0 / PAL3_s_drvinfo.width - 1.0)
#define y2gby(y) (1.0 - (y) * 2.0 / PAL3_s_drvinfo.height)
#define dx2gbdx(dx) ((dx) * 2.0 / PAL3_s_drvinfo.width)
#define dy2gbdy(dy) (-(dy) * 2.0 / PAL3_s_drvinfo.height)
#define scale_gbxdiff(xdiff, scalefactor) ((double)(xdiff) * (scalefactor) * GAME_WIDTH_ORG / PAL3_s_drvinfo.width)
#define scale_gbydiff(ydiff, scalefactor) ((double)(ydiff) * (scalefactor) * GAME_HEIGHT_ORG / PAL3_s_drvinfo.height)
#define pUIWND(x) ((struct UIWnd *)(x))
#define gbmalloc ((malloc_funcptr_t) (gboffset + 0x100C5C87))
#define gbfree ((free_funcptr_t) (gboffset + 0x100C5D39))


// PAL3A functions
#define pal3amalloc ((malloc_funcptr_t) TOPTR(0x00541A65))
#define pal3afree ((free_funcptr_t) TOPTR(0x005404C9))
#define PrepareDir ((int (*)(void)) TOPTR(0x00523059))

// global variables
#define GB_GfxMgr (*(struct gbGfxManager_D3D **) TOPTR(0x00C01CD4))
#define PAL3_s_drvinfo (*(struct gbGfxDriverInfo *) TOPTR(0x00C01788))



// structure selfcheck
#define PAL3A_STRUCT_SELFCHECK() do { \
    assert(sizeof(struct gbAudioMgrDesc) == 0x10C); \
    assert(sizeof(struct gbColorQuad) == 0x4); \
    assert(sizeof(struct UIWnd) == 0x44); \
    assert(sizeof(struct PtrArray) == 0x10); \
    assert(sizeof(struct gbMatrix4) == 0x40); \
    assert(sizeof(struct gbVec3D) == 0xC); \
    assert(sizeof(struct gbColorFlt) == 0x10); \
    assert(sizeof(struct gbViewPort) == 0x18); \
    assert(sizeof(struct gbClearParam) == 0x10); \
    assert(sizeof(struct gbGfxStatistic) == 0x30); \
    assert(sizeof(struct gbLight) == 0x68); \
    assert(sizeof(struct gbFogParam) == 0x34); \
    assert(sizeof(struct CD3DEnumeration) == 0x2C); \
    assert(sizeof(struct D3DDriverBug) == 0x4); \
    assert(sizeof(struct gbDynArray_pgbResource) == 0xC); \
    assert(sizeof(struct gbResManager) == 0xC); \
    assert(sizeof(struct CD3DSettings) == 0x6C); \
    assert(sizeof(struct gbGfxDriverInfo) == 0xB8); \
    assert(sizeof(struct gbGfxManager) == 0x5F0); \
    assert(sizeof(struct gbGfxManager_D3D) == 0x8F0); \
    assert(sizeof(struct gbUIQuad) == 0x28); \
    assert(sizeof(struct CPKHeader) == 0x80); \
    assert(sizeof(struct CPKTable) == 0x1C); \
} while (0)




#endif


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

// self-written functions
extern enum gbPixelFmtType gbGfxManager_D3D_GetBackBufferFormat(struct gbGfxManager_D3D *this);
extern int gbGfxManager_D3D_GetBackBufferBitCount(struct gbGfxManager_D3D *this);
extern void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus);
extern void *vfs_readfile(const char *filepath, unsigned *length, const struct memory_allocator *mem_allocator);
extern const char *vfs_cpkname(void);


#endif
#endif
