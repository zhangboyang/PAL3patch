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

enum VertexProcessingType {
    SOFTWARE_VP,
    MIXED_VP,
    HARDWARE_VP,
    PURE_HARDWARE_VP
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

struct CD3DEnumeration {
    IDirect3D9 *m_pD3D;
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

struct gbResManager {
    struct gbResource **pBuffer;
    int MaxNum;
    int CurNum;
};

struct gbGfxManager_D3D {
    char padding1[0xC];
    struct gbGfxDriverInfo DrvInfo;
    struct gbVertPoolMgr *pVertPoolMgr;
    struct gbEffectMgr *pEffectMgr;
    struct gbResManager *pTexResMgr;
    struct gbPrintFontMgr *pFontMgr;
    struct gbMatrixStack *pMatrixStack[0x4];
    char padding2[0x50C];
    struct CD3DEnumeration m_d3dEnumeration;
    int m_bWindowed;
    RECT m_rcWindowClient;
    RECT m_rcWindowBounds;
    ULONG m_dwWindowStyle;
    HWND m_hWnd;
    BYTE m_bMinimized;
    BYTE m_bMaximized;
    BYTE m_bClipCursorWhenFullscreen;
    BYTE m_bShowCursorWhenFullscreen;
    BYTE m_bActive;
    ULONG m_dwCreateFlags;
    struct D3DDriverBug m_DriverBug;
    struct gbResManager m_CursorMgr;
    struct gbCursorRes *m_pActiveCursor;
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
    struct UIWndVtbl *vfptr;
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
struct UIWndVtbl {
    void (__fastcall *ShowWindow)(struct UIWnd *this, int dummy, int);
    void (__fastcall *Render)(struct UIWnd *this, int dummy);
    int (__fastcall *Update)(struct UIWnd *this, int dummy, float, int);
    void (__fastcall *Destroy)(struct UIWnd *this, int dummy);
    void (__fastcall *Create)(struct UIWnd *this, int dummy, unsigned int, RECT *, struct UIWnd *, char);
    void *scalar_deleting_destructor;
    int (__fastcall *OnMessage)(struct UIWnd *this, int dummy, unsigned int, unsigned int, unsigned int);
};
#define UIWnd_vfptr_Render(this) THISCALL_WRAPPER((this)->vfptr->Render, this)

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
struct gbPrintFontMgr;

enum gbFontType {
    GB_FONT_UNICODE12,
    GB_FONT_UNICODE16,
    GB_FONT_UNICODE20,
    GB_FONT_NUMBER,
    GB_FONT_ASC,
};

struct gbPrintFont {
    struct gbPrintFontVtbl *vfptr;
    struct FontPrintInfo *ptInfo;
    struct FontSortInfoObj *pSortInfo;
    int maxInfo;
    int numInfo;
    struct FontPrint3DInfo *pt3DInfo;
    int num3DInfo;
    int max3DInfo;
    char *strBuffer;
    int maxStrBuffer;
    int curStrLoc;
    struct gbColorQuad curColor;
    float ScaleX;
    float ScaleY;
    float PitchX;
    float PitchY;
    float ZValue;
    struct gbRenderEffect *pEffect[2];
};

struct gbPrintFont_UNICODE {
    struct gbPrintFont;
    DWORD gap4C[24];
    int fontsize;
    DWORD gapB0[5];
};


struct gbBinkVideo;


struct C2DSpark_tagSpark {
    float fx;
    float fy;
    float fVx;
    float fVy;
    float fAx;
    float fAy;
    float fLum;
    float fLumInc;
    float fSize;
    float fSizeInc;
    char byColor;
    char bValid;
};

struct C2DSpark {
    struct C2DSpark_tagSpark m_Spark[512];
    float m_fDisplayWidth;
    float m_fDisplayHeight;
    int m_nCurrentSpark;
    float m_fBigLum;
    int m_nBigLumX;
    int m_nBigLumY;
    char m_bMouseTrack;
    struct gbRenderEffect *m_pAdditiveBlend;
    struct gbTexture *m_pRedBig;
    struct gbTextureArray *m_pRedBigArray;
    struct gbTexture *m_pBlue16;
    struct gbTextureArray *m_pBlue16Array;
    struct gbTexture *m_pGreen16;
    struct gbTextureArray *m_pGreen16Array;
    struct gbTexture *m_pRed16;
    struct gbTextureArray *m_pRed16Array;
    struct gbTexture *m_pYellow16;
    struct gbTextureArray *m_pYellow16Array;
    struct gbUIQuad m_Quad;
};

enum ECBShowItemKind {
    CBSIK_Null,
    CBSIK_Txt,
    CBSIK_RoleState,
    CBSIK_PrintTxt,
    CBSIK_Pic,
    CBSIK_FiveLineup,
    CBSIK_EnemyFiveLineup,
    CBSIK_CombatDialog,
    CBSIK_ScreenQuad,
    CBSIK_End,
};

struct tagShowItem {
    enum ECBShowItemKind eKind;
    char sTxt[256];
    RGBQUAD color;
    float fX;
    float fY;
    float fZ;
    float fSize;
};

struct UIStatic {
    struct UIWnd baseclass;
    int m_bordersize;
    int m_align;
    float m_ratiow;
    float m_ratioh;
    int m_textx;
    int m_texty;
    enum gbFontType m_fonttype;
    DWORD m_text[4]; // should be std::basic_string<char,std::char_traits<char>,std::allocator<char> >
    struct gbTexture *m_pbk;
    struct _Texture_Info *m_pbkInfo;
    struct gbTexture *m_ppic;
    struct _Texture_Info *m_ppicInfo;
    struct gbTexture *m_disablepic;
    struct _Texture_Info *m_disablepicInfo;
    struct gbTexture *m_pbk2;
    char m_mouseoff;
    int m_FVF;
    int m_nTextW;
};

struct UIFrameWnd {
    struct UIWnd baseclass;
    struct gbTexture *m_pbktex;
};

struct CCBUI;

struct CCBUI_funcptr_t {
    unsigned long (__fastcall *fp)(struct CCBUI *this, int dummy, unsigned long);
    DWORD fp_data[3];
};

struct tagUIParam {
    struct UIWnd *pUI;
    float fFromX;
    float fFromY;
    float fEndX;
    float fEndY;
    float fSpeed;
    float fEnergy;
    int nTimes;
};

struct tagUITask {
    struct CCBUI_funcptr_t fp;
    char bValid;
    char bExecuted;
    char bFinished;
    unsigned int dwID;
    float fStartTime;
    float fExpectStartTime;
    float fExpectEndTime;
    struct tagUIParam param;
    DWORD padding;
};

struct CCBUI {
    struct UIFrameWnd baseclass;
    struct UIStatic *m_pAttackSequenBack;
    struct UIStatic *m_pAttackSequenFace[11];
    struct UIStatic *m_pRoleStatePanel[4];
    struct UIStatic *m_pRoleStateFace[5];
    struct UIStatic *m_pRoleStateFaceName[5];
    union {
        struct {
            struct UIStatic *m_pRoleStateAttackInc[11];
            struct UIStatic *m_pRoleStateAttackDec[11];
            struct UIStatic *m_pRoleStateDefenceInc[11];
            struct UIStatic *m_pRoleStateDefenceDec[11];
            struct UIStatic *m_pRoleStateLuckInc[11];
            struct UIStatic *m_pRoleStateLuckDec[11];
            struct UIStatic *m_pRoleStateSpeedInc[11];
            struct UIStatic *m_pRoleStateSpeedDec[11];
            struct UIStatic *m_pRoleStateStable[11];
            struct UIStatic *m_pRoleStateBlank[11];
            struct UIStatic *m_pRoleStateForbid[11];
            struct UIStatic *m_pRoleStateSleep[11];
            struct UIStatic *m_pRoleStateChaos[11];
            struct UIStatic *m_pRoleStateMad[11];
            struct UIStatic *m_pRoleStateMirror[11];
            struct UIStatic *m_pRoleStateWall[11];
            struct UIStatic *m_pRoleStateBound[11];
            struct UIStatic *m_pRoleStateHermit[11];
            struct UIStatic *m_pRoleStateImmunity[11];
        };
        struct UIStatic *m_pRoleSpecState[19][11];
    };
    struct UIStatic *m_pWinPanel;
    struct UIStatic *m_pLosePanel;
    struct UIProgressBar *m_pRoleStateHP[4];
    struct UIProgressBar *m_pRoleStateGP[4];
    struct UIProgressBar *m_pRoleStateMP[4];
    struct UIProgressBar *m_pRoleStateSword;
    struct UIStaticCB *m_pTrickName;
    struct UIStaticCB *m_pTrickName2;
    struct UIStatic *m_pDialogBack;
    struct UIStatic *m_pResultLevelup;
    struct UIStatic *m_pResultWindLvup;
    struct UIStatic *m_pResultThunderLvup;
    struct UIStatic *m_pResultWaterLvup;
    struct UIStatic *m_pResultFireLvup;
    struct UIStatic *m_pResultEarthLvup;
    struct UIStatic *m_pResultDoubleLvup;
    struct UIStatic *m_pResultGetItem;
    struct CCBControlWindow *m_pMain;
    struct CCBItemWindow *m_pItemWindow;
    struct CCBMagicWindow *m_pMagicWindow;
    struct CCBSkillWindow *m_pSkillWindow;
    struct CCBAIWindow *m_pAIWindow;
    struct CCBLineupWindow *m_pLineupWindow;
    struct CCBProtectWindow *m_pProtectWindow;
    struct CCBResultWindow *m_pResultWindow[4];
    struct CCBSystem *m_pCBSystem;
    struct CUtil *m_pUtil;
    struct tagUITask m_Task[20];
    struct CCBUI_funcptr_t fp;
    int m_nTask;
    int m_nCurTask;
};




// functions
#define gbGfxManager_D3D_Reset3DEnvironment(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001AC50, int, struct gbGfxManager_D3D *), this)
#define gbGfxManager_D3D_BuildPresentParamsFromSettings(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001A190, void, struct gbGfxManager_D3D *), this)
extern enum gbPixelFmtType gbGfxManager_D3D_GetBackBufferFormat(struct gbGfxManager_D3D *this);
extern void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus);
#define UIWnd_SetRect(this, rect) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00445FA0, void, struct UIWnd *, RECT *), this, rect)
#define gbTexture_D3D_CreateForRenderTarget(this, width, height, format) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001BF80, int, struct gbTexture_D3D *, int, int, enum gbPixelFmtType), this, width, height, format)
#define gbTexture_D3D_CreateFromFileMemory(this, data, len) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001C010, void, struct gbTexture_D3D *, void *, int), this, data, len)
#define gbPrintFontMgr_GetFont(this, fonttype) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10004450, struct gbPrintFont *, struct gbPrintFontMgr *, enum gbFontType), this, fonttype)
#define gbPrintFont_PrintString(this, str, x, y, endx, endy) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10022BF0, void, struct gbPrintFont *, const char *, float, float, float, float), this, str, x, y, endx, endy)
#define gbmalloc ((malloc_funcptr_t) (gboffset + 0x100E4B0D))
#define gbfree ((free_funcptr_t) (gboffset + 0x100E4B99))
#define gbx2x(gbx) (((gbx) + 1.0) * gfxdrvinfo.width / 2.0)
#define gby2y(gby) ((1.0 - (gby)) * gfxdrvinfo.height / 2.0)
#define x2gbx(x) ((x) * 2.0 / gfxdrvinfo.width - 1.0)
#define y2gby(y) (1.0 - (y) * 2.0 / gfxdrvinfo.height)

// PAL3 functions
#define PrepareDir ((int (*)(void)) TOPTR(0x00538320))
#define gbBinkVideo_Width(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0053C710, int, struct gbBinkVideo *), this)
#define gbBinkVideo_Height(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0053C720, int, struct gbBinkVideo *), this)
#define gbBinkVideo_DrawFrameEx(this, pDestBuf, nDestPitch, nDestHeight, nDestLeft, nDestTop, nDestSurfaceType) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0053C530, int, struct gbBinkVideo *, void *, int, int, int, int, int), this, pDestBuf, nDestPitch, nDestHeight, nDestLeft, nDestTop, nDestSurfaceType)
#define PAL3_InitGFX ((void (*)(void)) TOPTR(0x00404FF0))
#define gbGfxManager_D3D_EndScene(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10018F00, void, struct gbGfxManager_D3D *), this)
#define UIDrawTextEx ((void (*)(const char *, RECT *, struct gbPrintFont *, int, int)) TOPTR(0x00541210))
#define UIPrint ((void (*)(int, int, char *, struct gbColorQuad *, int)) TOPTR(0x00540FD0))
#define C2DSpark_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004D61C0, void, struct C2DSpark *), this)
#define C2DSpark_CreateSingle(this, pSpark) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004D5EC0, bool, struct C2DSpark *, struct C2DSpark_tagSpark *), this, pSpark)
#define C2DSpark_CreateStars(this, x, y, nWidth, fStarSize) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004D6050, void, struct C2DSpark *, int, int, int, float), this, x, y, nWidth, fStarSize)
#define CCBUI_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x005163D0, bool, struct CCBUI *), this)
#define UIFrameWnd_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0043DDF0, void, struct UIFrameWnd *), this)
#define UIWnd_MoveWindow(this, x, y) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00445D20, void, struct UIWnd *, int, int), this, x, y)

// global variables
#define gfxdrvinfo (*(struct gbGfxDriverInfo *) TOPTR(0x00BFD6C8))
#define game_state (*(int *) TOPTR(0x00BFDA6C))
#define is_window_active (*(int *) TOPTR(0x005833B8))
#define config_flags (*(int *) TOPTR(0x005833BC))
#define g_GfxMgr (*(struct gbGfxManager_D3D **) 0x00BFDA60)
#define g_pVFileSys (*(struct gbVFileSystem **) (gboffset + 0x1015D3A8))


#endif
