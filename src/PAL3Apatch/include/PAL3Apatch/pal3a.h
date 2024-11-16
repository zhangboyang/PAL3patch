#ifndef PAL3APATCH_PAL3A_H
#define PAL3APATCH_PAL3A_H
// PATCHAPI DEFINITIONS


extern PATCHAPI void *load_image_bits(void *filedata, unsigned filelen, int *width, int *height, int *bitcount, const struct memory_allocator *mem_allocator);
extern PATCHAPI void ensure_cooperative_level(int requirefocus);
extern PATCHAPI void make_area_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom);
extern PATCHAPI void make_border_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom);
extern PATCHAPI int is_area_transparent(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom);
extern PATCHAPI void get_solid_area(void *bits, int width, int height, int bitcount, int pitch, int *left, int *top, int *right, int *bottom);
extern PATCHAPI void clamp_rect(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom);
extern PATCHAPI void copy_bits(void *dst, int dst_pitch, int dst_x, int dst_y, void *src, int src_pitch, int src_x, int src_y, int width, int height, int bitcount);
extern PATCHAPI void fill_texture(IDirect3DTexture9 *tex, D3DCOLOR color);
extern PATCHAPI void set_d3dxfont_matrices(IDirect3DDevice9 *pd3dDevice);

struct rawcpk;
extern PATCHAPI struct rawcpk *rawcpk_open(const char *cpkpath);
extern PATCHAPI void rawcpk_close(struct rawcpk *rcpk);
extern PATCHAPI void *rawcpk_read(struct rawcpk *rcpk, unsigned keycrc, unsigned *packedsize);
extern PATCHAPI char *rawcpk_hash(struct rawcpk *rcpk, unsigned keycrc, char *buf);


#ifdef USE_PAL3A_DEFINITIONS

struct member_function_pointer {
    union {
        struct {
            void *fp;
            BYTE gap4[12];
        };
        struct {
            unsigned long long gap0[2]; // force align
        };
    };
};

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

struct UIWndVtbl {
    MAKE_THISCALL(void, *ShowWindow, struct UIWnd *, int);
    MAKE_THISCALL(void, *Render, struct UIWnd *);
    MAKE_THISCALL(int, *Update, struct UIWnd *, float, int);
    MAKE_THISCALL(void, *Destroy, struct UIWnd *);
    MAKE_THISCALL(void, *Create, struct UIWnd *, unsigned int, RECT *, struct UIWnd *, char);
    void *scalar_deleting_destructor;
    MAKE_THISCALL(int, *OnMessage, struct UIWnd *, unsigned int, unsigned int, unsigned int);
};
#define UIWnd_vfptr_Render(this) THISCALL_WRAPPER((this)->vfptr->Render, this)
#define UIWnd_vfptr_Update(this, deltatime, haveinput) THISCALL_WRAPPER((this)->vfptr->Update, this, deltatime, haveinput)

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
    HWND hgfxwnd;
    int newwindow;
    int waitforverticalblank;
};

struct gbDynArray_pgbResource {
    struct gbResource **pBuffer;
    int MaxNum;
    int CurNum;
};


struct gbRefObject {
    struct gbRefObjectVtbl *vfptr;
    int RefCount;
};

struct gbResource {
    struct gbRefObject;
    char *pName;
    unsigned int NameCrc32;
    int IsLoaded;
    struct gbResManager *pMgr;
};

struct gbResManager {
    struct gbDynArray_pgbResource;
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
    struct gbColorQuad color;
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


enum ECPKMode {
    CPKM_Null = 0x0,
    CPKM_Normal = 0x1,
    CPKM_FileMapping = 0x2,
    CPKM_Overlapped = 0x3,
    CPKM_End = 0x4,
};

#define tagCPKHeader CPKHeader
#define tagCPKTable CPKTable

struct CPK {
    unsigned int m_dwAllocGranularity;
    enum ECPKMode m_eMode;
    struct tagCPKHeader m_CPKHeader;
    struct tagCPKTable m_CPKTable[32768];
    struct gbVFile *m_pgbVFile[8];
    bool m_bLoaded;
    unsigned int m_dwCPKHandle;
    unsigned int m_dwCPKMappingHandle;
    char m_szCPKFileName[260];
    int m_nOpenedFileNum;
};

struct gbBinkVideo {
    struct gbBinkVideoVtbl *vfptr;
    struct tagPOINT m_pos;
    struct BINK *m_hBink;
    struct CPK m_Cpk;
    struct CPK m_Cpk2;
};

enum GAME_STATE {
    GAME_NONE = 0x0,
    GAME_UI = 0x1,
    GAME_SCENE = 0x2,
    GAME_COMBAT = 0x3,
    GAME_CATCHGHOST = 0x4,
    GAME_WOLF = 0x5,
    GAME_HIDE_FIGHT = 0x6,
    GAME_COMPDONATE = 0x7,
    GAME_OVER = 0x8,
};

struct D3DAdapterInfo {
    int AdapterOrdinal;
    D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
    struct CArrayList *pDisplayModeList;
    struct CArrayList *pDeviceInfoList;
};

enum CURSOR_TYPE {
    CURSOR_NORMAL = 0x0,
    CURSOR_TALK = 0x1,
    CURSOR_CHECK = 0x2,
    CURSOR_NUM = 0x3,
};

struct UICursor {
    unsigned int m_dwID[3];
    enum CURSOR_TYPE m_active;
    struct gbTexture *m_tex[3];
    char m_bShow;
    char m_bSoftMode;
};

struct gbQuaternion {
    float x;
    float y;
    float z;
    float w;
};

struct gbCamera {
    float fov;
    float OrthoSize;
    float Wscreen;
    float Hscreen;
    float zNear;
    float zFar;
    struct gbViewPort Viewport;
    float Plane[6][4];
    int numPlane;
    struct gbMatrix4 ViewMatrix;
    struct gbMatrix4 InvVMatrix;
    struct gbCamControler *pControl[16];
    int CurCtrl;
    struct gbGfxManager *pGfxMgr;
    struct gbVec3D eyeLoc;
    struct gbQuaternion eyeDir;
};

struct GRP_KEYREG {
    int Data;
    int HoldTime;
    int CountMode;
    int PressTime;
};

struct GRPinput {
    IDirectInput8A *m_lpDI;
    IDirectInputDevice8A *m_lpDIDKeyboard;
    IDirectInputDevice8A *m_lpDIDMouse;
    float m_mouseSensitivity;
    int m_mouseMinX;
    int m_mouseMinY;
    int m_mouseMaxX;
    int m_mouseMaxY;
    int m_joystickMinX;
    int m_joystickMinY;
    int m_joystickMaxX;
    int m_joystickMaxY;
    POINT m_mousept;
    int m_mouseX;
    int m_mouseY;
    int m_mouseFreeX;
    int m_mouseFreeY;
    int m_mouseDeltaX;
    int m_mouseDeltaY;
    int m_joystickX;
    int m_joystickY;
    int m_joystickFreeX;
    int m_joystickFreeY;
    int m_joystickDeltaX;
    int m_joystickDeltaY;
    BYTE m_keyStates[270];
    BYTE m_keyRaw[256];
    unsigned int m_keyPressTimes[270];
    unsigned int m_keyDragStartPositions[270][2];
    BYTE m_shiftedKeyStates[270];
    unsigned int m_DIKToKEY[256];
    DIMOUSESTATE2 MouseState;
    unsigned int charTOscan[256];
    int KeyTransformation[10];
    int m_bMouse;
    int m_bKeyboard;
    int m_bJoystick;
    struct GRP_KEYREG KeyInfo[270];
};

enum ECBStageLockFlag {
    CBSLF_Null = 0x0,
    CBSLF_SkillAct = 0x1,
    CBSLF_MagicWait = 0x2,
    CBSLF_MagicAct = 0x3,
    CBSLF_ItemAct = 0x4,
    CBSLF_FlingAct = 0x5,
    CBSLF_BFAAct = 0x6,
    CBSLF_End = 0x7,
};

struct tagAttackSequen {
    float fDeltaTime;
    float fLastTime;
    int nIndex;
    unsigned int dwID;
    struct UIStatic *pPic;
    int nSlot1;
    int nSlot2;
    int nRound;
    BYTE bySpeed1;
    BYTE bySpeed2;
    bool bSlot2Running;
    bool bPaused;
    bool bValid;
};

struct CCBAttackSequen {
    struct CCBSystem *m_pCBSystem;
    struct CCBUI *m_pUI;
    struct CCBRoleState *m_pRole;
    struct tagAttackSequen m_Sequen[11];
    bool m_bEnable;
    bool m_bPause;
    bool m_bVisible;
    bool m_bLocked;
    enum ECBStageLockFlag m_eLockFlag[11];
};

enum ECBCombatState {
    CBCS_Null = 0x0,
    CBCS_Deactive = 0x1,
    CBCS_Running = 0x2,
    CBCS_Paused = 0x3,
    CBCS_Flee = 0x4,
    CBCS_Counting = 0x5,
    CBCS_FleeCounting = 0x6,
    CBCS_LoseCounting = 0x7,
    CBCS_CombatEnd = 0x8,
    CBCS_End = 0x9,
};
enum ECBCombatResult {
    CBCR_Null = 0x0,
    CBCR_NoResult = 0x1,
    CBCR_Win = 0x2,
    CBCR_Lose = 0x3,
    CBCR_FleeOK = 0x4,
    CBCR_End = 0x5,
};

struct tagCmdData {
    unsigned int dwCmdKind;
    union {
        unsigned int dwCmdID;
        unsigned int dwItemID;
    };
    int nSrcRole;
    int nDestRole;
    bool bMultiSrcRole;
    bool bMultiDestRole;
    bool bSrcRole[11];
    bool bDestRole[11];
    bool bValid;
};

struct tagThread {
    struct member_function_pointer fp;
    unsigned int dwID;
    int nTaskIndex;
    unsigned int dwTaskID;
    unsigned int dwReturn;
    int nTemp;
    bool bValid;
    bool bExecuted;
    struct tagCmdData cmd;
};

struct tagPlayerSet {
    struct tagCmdData LastRoleCmd;
    int nMagicDefaultPage;
    int nItemDefaultPage;
};

struct CCBSystem {
    struct CUtil *m_pUtil;
    struct CCBUI *m_pUI;
    struct CCBControl *m_pControl;
    struct CCBAttackSequen *m_pAttackSequen;
    struct CCBRoleState *m_pRoleState;
    struct CCBStage *m_pStage;
    struct CCBAI *m_pAI;
    struct CCBEditor *m_pEditor;
    struct CEffectSystem *m_pES;
    struct CEffectSimple *m_pEffectSimple;
    struct TxtFile *m_pConfig;
    struct CEffMgr *m_pEffMgr;
    struct CEffMgr *m_pATSEffMgr;
    struct C2DSpark *m_p2DSpark;
    struct C3DSpark *m_p3DSpark;
    double m_DeltaTime;
    double m_dTimeFund;
    float m_fTimeFund;
    double m_AbsDeltaTime;
    double m_dAbsTimeFund;
    float m_fAbsTimeFund;
    float m_fTimeScale;
    struct FightInfo *m_pFightInfo;
    bool m_bQuitFlag;
    bool m_bPause;
    enum ECBCombatState m_eState;
    enum ECBCombatResult m_eResult;
    char m_bFiveNimbus;
    char m_bPXVariance;
    int m_nTimesOfCombat;
    bool m_bAuto;
    bool m_bEditor;
    bool m_bLockCam;
    bool m_bFirstRender;
    unsigned int m_dwIDHolder;
    int m_nThread;
    int m_nCurThread;
    struct tagThread m_Thread[1024];
    struct tagPlayerSet m_Player[11];
};

typedef struct BINK BINK, *HBINK;



struct std_basic_string { // std::basic_string<char,std::char_traits<char>,std::allocator<char> >
    char allocator;
    char *_Ptr;
    unsigned int _Len;
    unsigned int _Res;
};

struct std_vector_int {
    char allocator;
    int *_First;
    int *_Last;
    int *_End;
};

struct std_vector_SoundItem {
    char allocator;
    struct SoundItem *_First;
    struct SoundItem *_Last;
    struct SoundItem *_End;
};

struct TxtFile {
    char m_token[512];
    char *m_buf;
    char *m_sectionstart;
    char *m_curpos;
};

struct SoundMgr {
    struct SoundMgrVtbl *vfptr;
    char m_bScriptMusic;
    struct std_basic_string m_szScriptMusic;
    int m_nScriptLoop;
    char m_szSceneMusic[256];
    struct TxtFile m_MusicTable;
    struct gbAudioManager *m_audiodrv;
    float DEF3DMasterVol;
    float DEF2DMasterVol;
    float DEFMusicMasterVol;
    char m_bValidDetectSceneMusic;
    struct std_vector_SoundItem m_cbbuf;
    int m_cbcursor;
    struct CPK m_Cpk;
};

typedef struct _SAMPLE *HSAMPLE;
typedef struct _STREAM *HSTREAM;
typedef struct _DIG_DRIVER *HDIGDRIVER;
typedef struct h3DPOBJECT *H3DPOBJECT;

struct SoundAttachObj {
    union {
        struct gbSound3DNode *p3d;
        struct gbSound2DNode *p2d;
    };
    union {
        H3DPOBJECT h3d;
        HSAMPLE h2d;
    };
    unsigned int time;
};

struct gbAudioManager {
    HSTREAM *hStreamArray[3];
    int StreamStatus[3];
    HDIGDRIVER *h2DDriver;
    unsigned int h3DProvider;
    struct SoundAttachObj Attach3D[10];
    struct SoundAttachObj Attach2D[4];
    unsigned int CurTime;
    float S_3DMasterVol;
    float S_2DMasterVol;
    float MusicMasterVol;
    H3DPOBJECT *hListener;
    struct gbVec3D ListenerPos;
    struct gbResManager SndDataMgr;
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

struct gbTexture {
    struct gbResource;
    int Width;
    int Height;
    struct gbImage2D *pTexImage;
    struct gbPixelFormat InFmt;
    int nLevels;
};
struct gbTexture_D3D {
    struct gbTexture;
    IDirect3DBaseTexture9 *pTex;
    IDirect3DSurface9 *pDS;
    unsigned int m_ImgFormat;
};

struct CTrail {
    float m_fTime;
    bool m_bSupport;
    bool m_bEnable;
    struct gbCamera *m_pCam;
    struct gbUIQuad m_ScreenQuad;
    unsigned int m_dwRenderCounter;
    unsigned int m_dwRT;
    char m_bReady;
    struct gbTexture_D3D m_texRT[8];
    struct gbRenderEffect *m_eft;
    struct IDirect3DSurface9 *m_OriginSurface;
};

struct gbTextureArray {
    struct gbTexture *pTexPt[16];
    int nTex;
};

enum gbPoolType {
    GB_POOL_STATIC = 0x0,
    GB_POOL_WRITEONLY = 0x1,
    GB_POOL_READONLY = 0x2,
};

struct gbDynVertBuf {
    struct gbDynVertBufVtbl *vfptr;
    enum gbPoolType Type;
    int VertNum;
    unsigned int VertType;
    int Stride;
    int BaseIndex;
    int LockVertNum;
    float *pXYZ;
    float *pNormal;
    struct gbColorQuad *pColor1;
    struct gbColorQuad *pColor2;
    float *pUV[4];
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

struct RenderTarget {
    int m_iMode;
    struct gbTexture_D3D m_Texture;
    struct gbTexture_D3D m_ScreenPlane;
    int m_nState;
};

struct gbCachedChinaFont {
    struct gbPrintFont;
    int nCacheTex;
    unsigned char *pFontImage;
    struct gbTexture *pCacheTex[16];
    int maxChars;
    unsigned char *pCharPoint;
    unsigned short *pAllChar;
    int nCacheSlot;
    struct gbCharCacheSlot *pCacheSlots;
    unsigned int iFrame;
    union {
        int FONT_SIZE;
        int fontsize;
    };
    int FONT_BITCOUNT;
    int FONT_BYTES;
    int CACHE_TEX_SIZE;
    float FONT_TEX_SIZE;
    int TEX_NCHAR;
};
#define gbPrintFont_UNICODE gbCachedChinaFont

enum gbFontType {
    GB_FONT_UNICODE12 = 0x0,
    GB_FONT_UNICODE16 = 0x1,
    GB_FONT_UNICODE20 = 0x2,
    GB_FONT_NUMBER = 0x3,
    GB_FONT_ASC = 0x4,
};

struct UI3DObj {
    bool m_mouseoff;
    int m_enable;
    int m_needdel;
    int m_type;
    union {
        struct gbGeomNode *m_pol;
        struct Actor *m_actor;
    };
    struct gbVec3D m_pos;
};

struct UI3DCtrl {
    struct UIWnd;
    int m_numobj;
    struct UI3DObj m_obj[5];
    float m_orthosize;
    int m_rotatemode;
    bool m_isrotateto;
    int m_speed;
    float m_rotate;
    float m_rotatetarget;
    float m_rotx;
    float m_rotz;
    float m_dropx;
    float m_dropy;
    float m_dropz;
    float m_scalex;
    float m_scaley;
    float m_scalez;
    struct gbLightObj *m_lightobj;
    struct gbCamera *m_camera;
    struct gbVec3D m_raydir;
};

struct UIStatic {
    struct UIWnd;
    int m_bordersize;
    int m_align;
    float m_ratiow;
    float m_ratioh;
    int m_textx;
    int m_texty;
    enum gbFontType m_fonttype;
    struct std_basic_string m_text;
    struct gbTexture *m_pbk;
    struct _PlugInfo *m_pbkInfo;
    struct gbTexture *m_ppic;
    struct _PlugInfo *m_ppicInfo;
    struct gbTexture *m_disablepic;
    struct _PlugInfo *m_disablepicInfo;
    struct gbTexture *m_pbk2;
    bool m_mouseoff;
    int m_FVF;
    int m_nTextW;
    bool m_PureColor;
    int m_richtext;
};

struct UIStaticEX {
    struct UIStatic;
    float m_rotate;
    int m_rotmode;
    float m_rotdst;
    float m_rotspeed;
    int m_rotdir;
    bool m_isStoped;
    float m_accelerate;
    float m_notespeed;
};

struct UIStaticEXA {
    struct UIStaticEX;
    int center_x;
    int center_y;
};

struct UnderWater {
    bool m_bEnable;
    IDirect3DIndexBuffer9 *m_pIB;
    IDirect3DVertexBuffer9 *m_pVB;
    IDirect3DTexture9 *m_pBumpmapTex;
    int m_iMode;
    float m_fTime;
};

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
    BYTE byColor;
    bool bValid;
    struct gbColorQuad color;
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
    struct tagRGBQUAD color;
    float fX;
    float fY;
    float fZ;
    float fSize;
};

struct UIFrameWnd {
    struct UIWnd;
    struct gbTexture *m_pbktex;
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
    struct member_function_pointer fp;
    bool bValid;
    bool bExecuted;
    bool bFinished;
    unsigned int dwID;
    float fStartTime;
    float fExpectStartTime;
    float fExpectEndTime;
    struct tagUIParam param;
};

struct CCBUI {
    struct UIFrameWnd;
    struct UIStatic *m_pAttackSequenBack;
    struct UIStatic *pPanel;
    struct UIStatic *m_pBattleFieldAttr[6];
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
    struct CCBParabolaWindow *m_pParabolaWindow;
    struct CCBMagicWindow *m_pMagicWindow;
    struct CCBSkillWindow *m_pSkillWindow;
    struct CCBAIWindow *m_pAIWindow;
    struct CCBLineupWindow *m_pLineupWindow;
    struct CCBProtectWindow *m_pProtectWindow;
    struct CCBResultWindow *m_pResultWindow[4];
    struct CCBFiveNimbusWindow *m_pFiveNimbusWindow;
    struct CCBSystem *m_pCBSystem;
    struct CUtil *m_pUtil;
    struct tagUITask m_Task[32];
    bool m_bShowResult;
    struct member_function_pointer m_fp;
    int m_nTask;
    int m_nCurTask;
    int mImbibeNimbusNum;
    struct tagImbibeNimbus *m_pImbibeNimbus;
};

struct CCBLineupWindow {
    struct UIFrameWnd;
    int m_nInitRoleFaceLineup[5];
    struct UIStatic *m_pBack;
    struct UIStatic *m_pFace[5];
    int m_nSelected;
    struct CCBUI *m_pUI;
};

enum ECBFiveNimbus {
    CBFN_Water = 0x0,
    CBFN_Fire = 0x1,
    CBFN_Thunder = 0x2,
    CBFN_Wind = 0x3,
    CBFN_Earth = 0x4,
    CBFN_Max = 0x5,
};

struct tagImbibeNimbus {
    struct UIStatic *mNimbus;
    enum ECBFiveNimbus mNimbustype;
    struct gbVec3D mVelocity;
    struct tagPOINT mPos;
    bool bAlive;
    float mAccelerate;
    float mFX;
    float mFY;
};

struct ObjectCamera {
    struct gbCamera *m_pCamera;
    struct gbVec3D m_camFrom;
    struct gbVec3D m_camTo;
    struct gbVec3D m_camUp;
    struct gbVec3D m_camDirect;
    float m_yaw;
    float m_pitch;
    float m_roll;
    float m_dist;
    float m_nyaw;
    float m_npitch;
    float m_nroll;
    struct gbVec3D m_forward;
    struct gbVec3D m_side;
    float m_stepRotSpeed;
    float m_stepRot;
    int m_stepRotMode;
    int m_stepRotAxis;
    bool m_bStepRot;
};

struct LineupUI;

struct LineupComp;

struct UIPar_Star {
    struct UIWnd;
    struct gbTexture *m_pTex;
    struct gbColorQuad m_color;
    int m_nFinalAlpha;
    int m_nMinAlpha;
    int m_nFade;
    int m_IsFadeRand;
    int m_IsFadeOut;
    int m_nLastTime;
    int m_IsLastTimeRand;
    int m_IsHighPoint;
    int m_IsInFlash;
    int m_nNextTime;
    int m_IsNextTimeRand;
    int m_nFrame;
};

struct UIPar_Meteor {
    struct UIWnd;
    struct gbTexture *m_pTex[2];
    struct gbColorQuad m_color;
    int m_nWidth;
    int m_nHeight;
    struct tagRECT m_StartToEnd;
    struct tagPOINT m_tmpPt;
    struct tagPOINT m_TailPt[16];
    int m_nTailNum;
    int m_ntmpTailNum;
    int m_nFrameNumBetweenTail;
    float m_fVelocityX;
    float m_fVelocityY;
    float m_passtime;
    int m_IsInMeteor;
    int m_nNextTime;
    int m_nFrame;
};

struct _CLOUD {
    struct UIStatic _cloud;
    struct tagRECT _cloudRc;
    float _cloudFX;
    int _width;
    int _height;
    float _cloudSpeed;
    float _alpha;
    float _fade;
    bool _isfade;
    int _up;
    int _yPt;
    int _fluctuate;
};


struct UITextureArray {
    struct gbTexture *m_ptex[16];
    int m_num;
};

enum UIButton_UIBUTTON_STATE {
    NORMAL = 0x0,
    MOUSEON = 0x1,
    MOUSEDOWN = 0x2,
    NUMSTATE = 0x3,
};

struct UIButton {
    struct UIWnd;
    bool m_hasDBclk;
    float m_ratiow;
    float m_ratioh;
    struct tagRECT m_drawrc;
    enum UIButton_UIBUTTON_STATE m_state;
    bool m_canctrl;
    bool m_bLockUpdate;
    int m_offsetx;
    int m_offsety;
    bool m_bBigFont;
    struct UITextureArray m_texarray;
    struct _PlugInfo *m_texarrayInfo[3];
    struct gbTexture *m_disablepic;
    struct _PlugInfo *m_disablepicInfo;
    bool m_mouseoff;
    bool m_mouseon;
    int m_alignmode;
    char m_caption[128];
    bool m_benable_snd;
};

struct _btnOPTDATA {
    char *pData;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
};

struct UIButtonEX {
    struct UIButton;
    struct _btnOPTDATA m_optData[3];
};


enum MENU_TYPE {
    MENU_COVER = 0x0,
    MENU_QCYM = 0x1,
    MENU_LEAVE = 0x2,
    MENU_JDHY = 0x3,
    MENU_SCENE = 0x4,
    MENU_YXXX = 0x5,
    MENU_XJTM = 0x6,
    MENU_ARCSAVE = 0x7,
    MENU_NONE = 0x8,
};

struct Average {
    unsigned int _size;
    float *_data;
    float ret;
    unsigned int _cur;
    unsigned int _real;
    bool _initialize;
    bool _invalid;
    bool _full;
};

#define UIStaticFlex_State UIButton_UIBUTTON_STATE
struct UIStaticFlex {
    struct UIWnd;
    float DURATION;
    struct std_basic_string m_text;
    struct tagRECT m_currect;
    struct UITextureArray m_texs;
    enum UIStaticFlex_State m_state;
    float m_statetime;
    enum gbFontType m_fonttype;
    int m_tilesize;
    int m_openstyle;
};

struct UIDialog {
    struct UIWnd;
    int m_bmodal;
};

enum DLG_TYPE {
    DLG_YES = 0x0,
    DLG_NO = 0x1,
    DLG_YESNO = 0x2,
    DLG_MSGBOX = 0x3,
    DLG_TYPENUM = 0x4,
};

enum DLG_MSG {
    DLG_MSG_NONE = 0xFFFFFFFF,
    DLG_BT_YES = 0x0,
    DLG_BT_NO = 0x1,
    DLG_MSG_END = 0x2,
    DLG_MSG_NUM = 0x3,
};

struct _TimeMgr {
    unsigned int newtick;
    unsigned int oldtick;
    bool bNote;
};

struct MUIDialog {
    struct UIDialog;
    //struct MUIDialogMgr;
    struct UIWnd *ptr_Dlg;
    unsigned int m_DlgId;
    struct tagRECT m_rect_MUIDialog;
    struct tagRECT m_textRc;
    struct UIStaticFlex m_bk;
    struct UIStatic m_static;
    struct UIStatic m_text;
    int m_btnW;
    int m_btnH;
    struct tagRECT m_btRc;
    struct UIButton mb_Yes;
    struct UIButton mb_No;
    int Lacune;
    enum DLG_TYPE m_type;
    enum DLG_MSG m_getMsg;
    int dft_W;
    int dft_H;
    bool m_IsModel;
    bool m_isDoModel;
    int m_xOffset;
    int m_yOffset;
    struct _TimeMgr _m_timeMgr;
    bool _m_canEndDlg;
    bool _haveSound;
};

struct UICoverFrm {
    struct UIFrameWnd;
    struct tagRECT rc;
    char buf[256];
    struct UIStatic m_background;
    struct UIPar_Star m_Star[13];
    struct UIPar_Meteor m_Meteor;
    struct _CLOUD m_cloud[10];
    struct _CLOUD m_midCloud[5];
    struct _CLOUD m_smlCloud[5];
    struct _CLOUD m_testCloud[9];
    struct UIStatic m_mountainA;
    struct UIStatic m_mountainB;
    struct UIStatic m_frame;
    struct UIStatic m_Logo;
    bool m_isAlpha;
    float m_alpha;
    struct UIStaticEXA m_tjA;
    struct UIStaticEXA m_tjB;
    struct UIButtonEX m_XDGS;
    struct UIButtonEX m_QCYM;
    struct UIButtonEX m_XJTM;
    struct UIButtonEX m_LKYX;
    struct UIButtonEX m_YXXX;
    struct UIButtonEX m_JDHY;
    struct UIButtonEX *m_CtrlBtn[6];
    int m_SelBtn;
    bool m_ExitGame;
    struct UITitle_QCYM *pQCYM;
    struct UITitle_JDHY *pJDHY;
    struct UITitle_YXXX *pYXXX;
    struct UITitle_XJTM *pXJTM;
    struct UITitle_ArcSave *pArcSave;
    struct UILoadingFrm *pLoadingFrm;
    enum MENU_TYPE m_MenuType;
    struct Average m_timeAverage;
    bool m_gotogame;
    unsigned int m_flag;
    struct UIStatic m_test;
    struct MUIDialog m_ArcDlg;
    struct MUIDialog m_Not1stDlg;
    bool m_goingin;
    bool m_ArcSaveSuc;
    bool m_IsReadFinishSave;
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

struct gbVFileSystem {
    int IsInit;
    char *rtDirectory;
    struct gbVFile *pFileBuffer;
    enum {
        VFSYS_DISKFILE = 0x0,
        VFSYS_PACKFILE = 0x1,
        VFSYS_MEMIMAGE = 0x2,
    } Type;
    struct _iobuf *m_pckfp;
    unsigned int m_itemcount;
    struct CPK m_cpk;
};

struct UIPosMaker {
    bool m_bIsLoaded;
    struct _UIPOS *m_pUIPos;
    int m_nPosNum;
};

struct ArtistPlugIn {
    struct _iobuf *pFile;
    char _libfolder[512];
    struct _PlugInfo *pInfo;
    unsigned int m_infoNum;
    bool m_alreadyLoad;
    struct tagRECT m_tRect;
    bool m_bOptmize;
    struct UIPosMaker m_posMaker;
};

struct _Texture_uv {
    float u;
    float v;
};

struct _PlugInfo {
    int filepos;
    char name[256];
    unsigned int crc;
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
    unsigned int srcWidth;
    unsigned int srcHeight;
    char libName[256];
    unsigned int libWidth;
    unsigned int libHeight;
    struct _Texture_uv _tex_uvLT;
    struct _Texture_uv _tex_uvLB;
    struct _Texture_uv _tex_uvRB;
    struct _Texture_uv _tex_uvRT;
};

struct HeadMsg {
    struct HeadMsgVtbl *vfptr;
    bool m_bEnable;
    struct std_basic_string m_szMsg;
    float m_fTime;
    int m_nType;
    int m_nY;
    struct UIStatic *m_pMsgDlg;
    int m_nMsgWidth;
    int m_nMsgHeight;
    int m_nShowPointW;
    float m_fHeadModeTime;
    float m_fAttenSpeed;
    bool m_bAtten;
    bool m_ucAlpha;
    struct Role *m_pRole;
};

struct PalScriptWaitObj {
    struct PalScriptWaitObjVtbl *vfptr_PalScriptWaitObjVtbl;
    int m_needscriptdel;
};

struct UIAnimateCtrl {
    struct UIWnd;
    float m_FPS;
    float m_passtime;
    struct UITextureArray m_texarray;
    int m_curframe;
    int m_numframe;
    int m_looppassed;
    int m_nLoop;
    int m_mode;
};

enum UIEmote_UIEMOTE {
    EM_NONE = 0x0,
    EM_SHUI = 0x1,
    EM_JING = 0x2,
    EM_YI = 0x3,
    EM_NU = 0x4,
    EM_XI = 0x5,
    EM_XIN = 0x6,
    EM_HAN = 0x7,
    EM_LUAN = 0x8,
    EM_JI = 0x9,
    EM_QI = 0xA,
    EM_YUN = 0xB,
    EM_SHENG = 0xC,
    EM_BEI = 0xD,
    EM_NUM = 0xE,
};

struct UIEmote {
    struct UIWnd;
    struct PalScriptWaitObj;
    enum UIEmote_UIEMOTE m_cur;
    struct UIAnimateCtrl m_ani[14];
    struct Role *m_role;
    int m_loopcount;
    int m_nLoop[14];
};

struct UICaption {
    struct UIWnd;
    struct PalScriptWaitObj;
    struct gbTexture *m_tex;
    int m_numline;
    int m_lineheight;
    int m_curline;
    float m_linetime;
    struct gbTexture *m_tex2;
    int m_numline2;
    int m_curline2;
    float m_linetime2;
    struct tagRECT m_rect2;
    bool m_bHas2;
    bool m_bDraw2;
};

struct UINote {
    struct UIDialog;
    struct UIStaticFlex m_static;
    float time;
};

enum UIFlexBar_STYLE {
    HORIZONTAL = 0x0,
    VERTICAL = 0x1,
};

struct UIFlexBar {
    struct UIWnd;
    int m_tilewidth;
    int m_tileheight;
    struct gbTexture *m_tex;
    enum UIFlexBar_STYLE m_style;
};

enum UIScrollBar_SB_AREA {
    UPBTN = 0x0,
    DOWNBTN = 0x1,
    SLIDER = 0x2,
    PAGEUP = 0x3,
    PAGEDOWN = 0x4,
    NONE = 0x5,
};

struct UIScrollBar {
    struct UIWnd;
    bool m_bPress;
    bool m_bPressed;
    struct UIFlexBar m_bk;
    int m_DragState;
    bool m_bDrag;
    struct tagPOINT m_lastPt;
    int m_btnH;
    enum UIScrollBar_SB_AREA m_mouseat;
    int m_min;
    int m_max;
    int m_pos;
    float m_lastpos;
    float m_drawpos;
    float m_step;
    int m_pagesize;
    struct UITextureArray m_texarray;
    struct _PlugInfo *m_texarrayInfo[10];
    float m_keytime;
    struct tagRECT m_btnrect[2];
    int m_slideroffset;
    int m_slidertboffset;
    int m_sliderw;
    int m_sliderh;
};

struct UILBColumn {
    char head[64];
    struct PtrArray items;
    int width;
    bool rightalign;
    int rightoffset;
};

struct UIListBox {
    struct UIWnd;
    struct gbColorQuad m_selcolor;
    int m_showGrayBk;
    bool m_hasKeyCtrl;
    struct UIScrollBar m_scrollbar;
    int m_numcol;
    struct UILBColumn m_column[8];
    int m_itemheight;
    int m_fontsize;
    int m_firstshow;
    int m_onsel;
    int m_locksel;
    struct tagRECT m_scrollRc;
    struct gbTexture *m_pbkpic;
    struct _PlugInfo *m_pbkpicInfo;
    struct gbTexture *m_selpic;
    struct _PlugInfo *m_selpicInfo;
    int m_seloffset;
    int m_seloffsetL;
    int m_seloffsetR;
    int m_scrollW;
    struct gbColorQuad m_seltexcolor;
    struct tagPOINT m_itemoffset;
    int m_movesel;
    struct std_vector_int m_rowenable;
};

struct UIRoleSelect {
    struct UIDialog;
    struct PalScriptWaitObj;
    int m_sel;
    int m_maxlen;
    struct UIListBox m_list;
    struct UIStaticFlex m_bk;
};

struct UIHeadDialog {
    struct UIDialog;
    struct PalScriptWaitObj;
    struct Role *m_role;
    int m_width;
    int m_height;
    struct gbTexture *m_bktex;
    char m_text[256];
};

enum _FLIP {
    STATIC_LEFT_FLIP = 0x0,
    STATIC_RIGHT_FLIP = 0x1,
};

struct UIStaticFlip {
    struct UIStatic;
    enum _FLIP flipMode;
};

struct UITextArea {
    struct UIWnd;
    struct std_basic_string m_buf;
    struct std_basic_string m_page;
    bool m_bInvalidate;
};

struct UIRoleDialog {
    struct UIDialog;
    struct PalScriptWaitObj;
    struct UIStatic m_bk;
    struct UIAnimateCtrl m_ani;
    struct UIStaticFlip m_face;
    struct UITextArea m_static;
    float m_fPosY;
    float m_SfPosY;
    bool m_bReady;
    int m_dlgh;
    struct tagRECT m_bkRc;
    int m_mode;
    int m_leftright;
    float m_time;
    int m_sel;
};

struct std_vector__Btn_BtnID {
    char allocator;
    struct _Btn_BtnID *_First;
    struct _Btn_BtnID *_Last;
    struct _Btn_BtnID *_End;
};

struct UITabBtn {
    struct std_vector__Btn_BtnID m_BtnAndID;
    int m_SelBtn;
    struct UIWnd *m_pFather;
};

struct TimeCtl {
    unsigned int newtick;
    unsigned int oldtick;
    bool bNote;
};

struct ScriptData {
    WORD m_ID;
    char m_Name[256];
    int m_RestPrice;
    char m_Rest[256];
    char m_Enough[256];
    char m_NoEnough[256];
    char m_RestEnd[256];
};

struct RestScript {
    struct TxtFile m_LoadRest;
    char m_lastFile[256];
    struct ScriptData m_Data;
};

struct UIChatRest {
    struct UIWnd;
    struct tagPOINT m_pt;
    struct tagRECT rc;
    struct tagRECT bkrc;
    int m_startX;
    int m_startY;
    int mbWidth;
    int mbHeight;
    struct UIStatic m_FlagT;
    struct UIStatic m_FlagB;
    struct UIStatic m_FlagM[3];
    struct UIButton m_Chat;
    struct UIButton m_Rest;
    struct UIButton m_Exit;
    struct UITabBtn m_TabBtn;
    struct MUIDialog m_answerDlg;
    bool m_bAnswer;
    struct ScreenEffect *eft;
    struct TimeCtl m_timewait;
    bool m_isFadeOut;
    bool m_isFadeIn;
    struct MUIDialog m_noenoughmsg;
    struct MUIDialog m_enough;
    struct MUIDialog m_restover;
    unsigned int m_status;
    bool m_isExit;
    struct RestScript m_RestScript;
    int m_CANREST_ID;
    int m_CANNOTREST_ID;
    int m_RESTOVER_ID;
    struct PalScript *pRest;
    struct PalScript *pCantRest;
    struct PalScript *pRestOver;
    bool m_bActive;
    struct Comp_DonateUI *pC_DUI;
    bool m_bC_DUIOver;
    struct UIStatic m_BK;
    bool m_FadeOutAlpha;
    float m_age;
    float m_life;
};

enum PAL3_FRAME {
    FRM_SCENE = 0x0,
    FRM_FIGHT = 0x1,
    FRM_DEAL = 0x2,
    FRM_COVER = 0x3,
    FRM_S_STATE = 0x4,
    FRM_COMBAT = 0x5,
    FRM_CATCHGHOST = 0x6,
    FRM_HS_ENTRY = 0x7,
    FRM_APPRAISE = 0x8,
    FRM_KF = 0x9,
    FRM_ENCAMPMENT = 0xA,
    FRM_SKEE = 0xB,
    FRM_ROWING = 0xC,
    FRM_BIGMAP = 0xD,
    FRM_LOADING = 0xE,
    FRM_GAMEOVER = 0xF,
    FRM_COMPOSE = 0x10,
    FRM_JUSTICEBOOK = 0x11,
    FRM_NUM = 0x12,
};

struct UIGameFrm {
    struct UIWnd;
    struct PalScriptWaitObj;
    unsigned int m_bigMapElement[16];
    bool m_newbigmap[16];
    struct UICaption m_cap;
    struct UINote m_note;
    unsigned int m_SmeltChatScript;
    unsigned int m_HockShopScript;
    unsigned int m_EncampFlag;
    unsigned int m_ExitRow;
    bool m_SDTY;
    bool m_APPR;
    bool m_SKEE;
    unsigned int m_ExitSkee;
    struct ScreenEffect *eft;
    bool m_isFadeOut;
    bool m_overFade;
    struct UIRoleSelect m_seldlg;
    struct UIHeadDialog m_headdlg[5];
    struct UIRoleDialog m_roledlg;
    struct UIChatRest m_ChatRest;
    unsigned int m_ChatRestStatus;
    struct UIFrameWnd *m_curfrm;
    enum PAL3_FRAME m_curfrmid;
    enum PAL3_FRAME m_lastfrmid;
    struct UIEmote m_emote[6];
    struct UISceneFrm *m_scenefrm;
    struct StateUI *m_sysfrm;
    struct UIStatic m_selemote_frame;
    struct UIButton m_selemote_BTN1;
    struct UIButton m_selemote_BTN2;
    int m_selemote_nSel;
    bool m_selemote_create;
    struct MUIDialog m_FinishSave;
};

enum UIProgressBar_PGR_MODE {
    NORMAL_PGR = 0x0,
    ALPHA_PGR = 0x1,
};

struct UIProgressBar {
    struct UIWnd;
    int m_mode;
    int m_dir;
    struct tagRECT m_ProgressRc;
    struct tagRECT m_decorateRc;
    float m_progress;
    float m_speed;
    float m_SetProgress;
    float m_Lenth;
    float m_MaxVal;
    float m_stepval;
    float m_radius;
    float m_lradius;
    float m_rradius;
    float m_tradius;
    float m_bradius;
    float m_step;
    float m_Val;
    bool m_IsGetit;
    struct gbColorQuad m_bkColor;
    struct gbColorQuad m_foColor;
    struct gbTexture *m_pbk;
    struct _PlugInfo *m_pbkInfo;
    struct gbTexture *m_ppic;
    struct _PlugInfo *m_ppicInfo;
    struct gbTexture *m_pdecorate;
    struct _PlugInfo *m_pdecorateInfo;
    int m_decorateW;
    int m_decorateH;
    int m_decorateOffsetX;
    int m_decorateOffsetY;
    enum UIProgressBar_PGR_MODE m_pgrmode;
    struct gbColorQuad m_color;
    int m_dstAlpha;
    int m_alpha;
    int m_minAlpha;
    float m_fadespeed;
    bool m_isAdd;
    bool m_isFlash;
    bool m_stop;
    int m_warningval;
};

struct UISceneMap {
    struct UIWnd;
    bool m_bLeft;
    bool m_bPlayer;
    float m_lastZ;
    struct gbVec3D m_lastLookAt;
    struct UIButton m_probe;
    struct UIStatic m_base;
    struct UIStatic m_addin;
    struct UIStaticEX m_compass;
    struct UIStaticFlex m_mapbk;
    struct UITextureArray m_icons;
    int m_state;
};

struct UISceneFace {
    struct UIWnd;
    struct UIProgressBar m_hp;
    struct UIProgressBar m_mp;
    struct UIStatic m_face[3];
    struct UIStatic m_bk;
    float m_clicktime;
    int m_onclick;
    struct UIStatic m_state[4];
};

struct UISceneFrm {
    struct UIFrameWnd;
    struct UISceneMap m_map;
    struct UISceneFace m_face;
};

struct UIGameOver {
    struct UIFrameWnd;
    struct gbTexture *m_pbktex_UIGameOver;
    struct UIStatic m_Blood;
    float m_fTime;
};

struct UIStaticVtbl {
    struct UIWndVtbl;
};



struct UIButtonEXA {
    struct UIButton;
    struct _btnOPTDATA m_optData;
    int m_EnableMode;
    struct gbColorQuad m_color;
    struct gbColorQuad m_enablecolor;
    struct gbColorQuad m_surecolor;
};

struct _MapElement {
    bool m_bShow;
    bool m_cango;
    struct tagPOINT m_screenPt;
    struct UIButtonEXA m_btn;
    bool m_isBtnCreate;
    bool m_show;
};

enum _BIGMAP_ELEMENT {
    _invalid = 0xFFFFFFFF,
    _tangjiabao = 0x0,
    _dengyunlu = 0x1,
    _shushan = 0x2,
    _dimaimenhu = 0x3,
    _lvluozhang = 0x4,
    _lvluoshan = 0x5,
    _lishushan = 0x6,
    _shengzhou = 0x7,
    _nalingheyuan = 0x8,
    _jingcheng = 0x9,
    _shicun = 0xA,
    _shushangudao = 0xB,
    _suoyaota = 0xC,
    _yuzhou = 0xD,
    _shidong = 0xE,
    _yuguangcheng = 0xF,
    _ELEMENT_NUM = 0x10,
};

struct UIBigMap {
    struct UIFrameWnd;
    struct tagRECT screenrect;
    int bkWndX;
    int bkWndY;
    int i;
    int j;
    char buf[256];
    struct UIStatic ms_BigMap;
    struct _MapElement m_Element[16];
    struct tagRECT m_spiritRect;
    struct UIAnimateCtrl m_spirit;
    float m_spiritPosX;
    float m_spiritPosY;
    struct tagPOINT m_spiritPos;
    struct tagPOINT m_movetopos;
    float m_movetoX;
    float m_movetoY;
    int m_spiritspeed;
    float m_speedFX;
    float m_speedFY;
    bool m_bSpiritCanMove;
    bool m_isSpiritMoveTo;
    float m_timescale;
    int m_stopdir;
    enum _BIGMAP_ELEMENT m_curElement;
    struct MUIDialog m_Answer;
    struct MUIDialog m_Msg;
    struct TxtFile scnname;
    bool m_haveopen;
    bool m_bupalpha;
    int m_alpha;
    int m_alphatime;
};


struct LEVELPARAM {
    int nMonster;
    int nSecond;
};

struct CG_Entry {
    bool m_bActive;
    bool m_bUIActive;
    struct CG_Scene *m_pScene;
    struct CG_UI *m_pUI;
    struct CG_Edit *m_pEdit;
    struct LEVELPARAM m_LEVELPARAM[5];
    enum PAL3_FRAME m_EntryFrame;
    struct gbVec3D m_EntryRolePos;
    char m_EntryCityName[64];
    char m_EntryName[64];
    bool b_NoteJumpFree;
};

enum UIS_RM_Mode {
    RM_DEFAULT = 0x0,
    RM_ALPHA = 0x1,
};
struct UIStaticRM {
    struct UIStatic;
    struct gbColorQuad m_color;
    int m_dstAlpha;
    int m_alpha;
    int m_minAlpha;
    float m_fadespeed;
    bool m_isAdd;
    bool m_isFlash;
    bool m_stop;
    enum UIS_RM_Mode m_mode;
};

struct st_Manager {
    bool m_Type;
    bool m_Dicker;
    unsigned short m_ID;
    char m_Name[256];
    bool m_C;
    int m_Patience;
    char m_BeginTalk[256];
    char m_EntryBuy[256];
    char m_EntrySale[256];
    char m_NormalAcptTalk[256];
    char m_NormalCnslTalk[256];
    char m_NoEnoughMoney[256];
    char m_Leave[256];
    char m_HarshTalkbuy[256];
    float m_AcptQuantum[5];
    char m_AcptTalk[5][2][256];
    float m_RfusQuantum[5];
    char m_RfusTalk[5][2][256];
    float m_HarshQuantum[5];
    char m_HarshTalk[5][2][256];
    char m_ForceAcptTalk[5][2][256];
    char m_ForceAcptTalkback[5][256];
    int m_StoreNum[2];
    unsigned int m_StoreID[2][200];
    struct FightItem *m_StoreData[2][200];
    bool m_IsCtrl[2][200];
};

struct CG_UI {
    struct UIFrameWnd;
    char buf[256];
    struct tagRECT m_ParentRc;
    struct CG_Entry *m_pFather;
    int m_SelType;
    int m_SelPropType;
    int m_nResultValue;
    int m_nWinState;
    struct UIStatic m_OutFrameT;
    struct UIStatic m_OutFrameR;
    struct UIStatic m_OutFrameL;
    struct UIStatic m_OutFrameB;
    struct UIStatic m_pTime;
    struct UIStatic m_pMoney;
    struct UIStatic m_pTools;
    struct UIButton m_pItems[9];
    struct UIStatic m_pName;
    struct UIStatic m_pPrice;
    struct UIStatic m_pHurt;
    struct UIStatic m_pInfo;
    struct UIButton m_pBuy;
    struct UIButton m_pRun;
    struct UIStatic m_pProp;
    struct UIButton m_pProps[5];
    struct UIStatic m_pPropsNum[5];
    struct UIStatic m_pHP;
    struct UIProgressBar m_pBossHP;
    struct UIProgressBar m_pRoleHP;
    struct UIStatic m_pRoleFace;
    struct UIStatic m_pBossFace;
    struct UIStatic m_pTimeBackground;
    struct UIStaticRM m_pTimeHunDigit;
    struct UIStaticRM m_pTimeTenDigit;
    struct UIStaticRM m_pTimeSinDigit;
    struct UIStatic m_pResult;
    struct MUIDialog m_pWinAnswer;
    struct MUIDialog m_pLoseAnswer;
    struct MUIDialog m_pNoMoneyMsg;
    struct MUIDialog m_pSurpassMsg;
    struct MUIDialog m_pAccountMsg;
    struct MUIDialog m_pOverGameMsg;
    struct MUIDialog m_pExitDlg;
    struct PtrArray m_ComboBuffer;
    struct UISceneMap m_SceneMap;
    struct UIButton m_Exit;
    bool m_IsExit;
    struct tagRECT m_InvalidRc0;
    struct tagRECT m_InvalidRc1;
    struct tagRECT m_InvalidRc2;
    int m_nTimeSec;
    int m_nBossHP;
    float m_fBossHP;
    int m_nRoleFullHP;
    int m_nRoleCurHP;
    float m_ComboHitLastTime;
    float m_ComboHitTime;
    int m_nComboHit;
    int m_nTotalComboHit;
    struct UIStatic m_pCombo;
    bool m_mouseon[9];
    int m_nPropCost;
    int m_nNoteMoney;
    struct PropUI *m_pBuyUI;
    bool m_bRenderBuyUI;
    struct st_Manager m_PropData;
    bool bUseKeyLocked;
    unsigned int m_nPropNum[5];
    unsigned int m_dwCurPropID;
};

enum _BGM {
    _BGM_UPDW = 0x0,
    _BGM_LR = 0x1,
};

struct BtnGroup {
    struct tagRECT upRc;
    struct tagRECT dwRc;
    struct UIButton m_btnUp;
    struct UIButton m_btnDw;
    int m_sizex;
    int m_sizey;
    enum _BGM m_mode;
    int m_Interval;
    bool m_opt;
};

struct PropUI {
    struct UIFrameWnd;
    int bkWndX;
    int bkWndY;
    int m_ListWidth;
    int m_ListHeight;
    struct st_Manager m_PropData;
    struct PlayerItem *p_DealData;
    char buf[256];
    char remarkbuf[256];
    int m_SelNum;
    int *m_ListSelNum;
    int *m_MaxNum;
    int m_x;
    int m_y;
    int m_Sum;
    int i;
    int j;
    bool _isCreatable;
    struct UIStatic m_Frame;
    struct UIFlexBar m_FrameBar;
    struct UIStatic m_Flag;
    struct UIStatic m_OutFrameT;
    struct UIStatic m_OutFrameR;
    struct UIStatic m_OutFrameL;
    struct UIStatic m_OutFrameB;
    struct tagRECT mb_ExitRc;
    struct UIButton mb_Exit;
    struct UIAnimateCtrl m_ani;
    struct UIStatic m_Role[5];
    struct tagRECT m_RoleRc[5];
    int RoleX;
    int RoleY;
    int HeadW;
    int HeadH;
    bool m_Is[5];
    struct MUIDialog m_Msg;
    struct UI3DCtrl m_3DObj;
    struct UIStatic m_Dlg;
    struct tagRECT m_DlgRc;
    struct UIStatic m_TalkFrame;
    struct UIStatic m_Ready;
    struct tagRECT m_ReadyRc;
    struct UIStatic m_ListFrame;
    struct UIStatic m_ListGrid[14];
    struct UIStatic m_ScrollHB;
    struct UIStatic m_EquipFlag;
    struct UIStatic m_ListPropTitle;
    struct tagRECT m_ListPropTitleRc;
    struct UIListBox ml_ListProp;
    struct tagRECT ml_ListPropRc;
    struct BtnGroup m_btnGroup;
    float m_ctrlTime;
    bool m_bCanUpdate1;
    bool m_bCanUpdate2;
    struct UIStatic m_PropRemarkBk;
    struct UIStatic m_PropRemark;
    struct UIStatic m_PropAtt;
    struct UIStatic m_MoneyRemark;
    struct tagRECT m_MoneyRc;
    struct UIStatic m_SumProp;
    struct tagRECT m_SumRc;
    struct UIButton mb_OK;
    struct tagRECT mb_OKRc;
    bool m_IsDeal;
    struct UIButton mb_Cancel;
    struct tagRECT mb_CancelRc;
    unsigned int *m_DealID;
    int DealNum;
    int curID;
    bool m_isBuy;
    bool b_IsExterior;
    int exterior_num;
    float discount_price;
    int m_extmode;
    struct DealUI *pDealUI;
    struct UITabBtn m_TabBtn;
    bool m_opt;
};

struct FindArc;

struct Archive;

struct RoleTaskLine {
    int nType;
    int ID;
    char bFinished;
    char taskpic[128];
};
struct std_vector_RoleTask {
    char allocator;
    struct RoleTask *_First;
    struct RoleTask *_Last;
    struct RoleTask *_End;
};
struct std_vector_RoleTaskLine {
    char allocator;
    struct RoleTaskLine *_First;
    struct RoleTaskLine *_Last;
    struct RoleTaskLine *_End;
};
struct RoleTaskPool {
    struct std_vector_RoleTask m_TaskData;
    struct std_vector_RoleTaskLine m_TaskLine;
    struct TxtFile m_taskFile;
    char _TaskPicFolder[128];
};

struct UIDragBar;


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
#define gbGfxManager_D3D_EndScene(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x100188E0, void, struct gbGfxManager_D3D *), this)
#define gbGfxManager_D3D_BuildPresentParamsFromSettings(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10019A70, void, struct gbGfxManager_D3D *), this)
#define gbGfxManager_D3D_Reset3DEnvironment(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001A480, int, struct gbGfxManager_D3D *), this)
#define gbCrc32Compute ((unsigned (*)(const char *)) TOPTR(gboffset + 0x10026710))
#define gbCamera_GetViewSizeOnNearPlane(this, hw, hh) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10021480, void, struct gbCamera *, float *, float *), this, hw, hh)
#define gbTexture_D3D_CreateForRenderTarget(this, width, height, format) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001B950, int, struct gbTexture_D3D *, int, int, enum gbPixelFmtType), this, width, height, format)
#define gbTexture_D3D_Ctor(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001B440, struct gbTexture_D3D *, struct gbTexture_D3D *), this)
#define gbTexture_D3D_Dtor(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001B490, void, struct gbTexture_D3D *), this)
#define gbCamera_SetDimention(this, a2, a3) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10021680, void, struct gbCamera *, int, int), this, a2, a3)
#define gbPrintFont_PrintString(this, str, x, y, endx, endy) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10022510, void, struct gbPrintFont *, const char *, float, float, float, float), this, str, x, y, endx, endy)
#define gbVertPoolMgr_GetDynVertBuf(this, a2) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10021150, struct gbDynVertBuf *, struct gbVertPoolMgr *, unsigned int), this, a2)
#define gbPrintFontMgr_GetFont(this, fonttype) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x100045F0, struct gbPrintFont *, struct gbPrintFontMgr *, enum gbFontType), this, fonttype)
#define gbVFileSystem_OpenFile(this, filename, mode) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1002DB10, struct gbVFile *, struct gbVFileSystem *, const char *, unsigned int), this, filename, mode)
#define gbVFileSystem_GetFileSize(this, fp) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1002E170, long, struct gbVFileSystem *, struct gbVFile *), this, fp)
#define gbVFileSystem_Read(this, buf, size, fp) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1002DD70, void, struct gbVFileSystem *, void *, unsigned int, struct gbVFile *), this, buf, size, fp)
#define gbVFileSystem_CloseFile(this, fp) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1002DCF0, void, struct gbVFileSystem *, struct gbVFile *), this, fp)
#define gbMatrixStack_Scale(this, a2, a3, a4) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10026460, void, struct gbMatrixStack *, float, float, float), this, a2, a3, a4)
#define gbMatrixStack_Translate(this, a2, a3, a4) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10026430, void, struct gbMatrixStack *, float, float, float), this, a2, a3, a4)
#define gbMatrixStack_Rotate(this, angle, axis) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10026350, void, struct gbMatrixStack *, float, struct gbVec3D *), this, angle, axis)
#define gbImage2D_WriteJpegImage(this, filename, quality) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x1001F0D0, void, struct gbImage2D *, char *, int), this, filename, quality)
#define gbAudioManager_Ctor(this, succeed, pdesc) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gboffset + 0x10001110, struct gbAudioManager *, struct gbAudioManager *, int *, struct gbAudioMgrDesc *), this, succeed, pdesc)




// PAL3A functions
#define pal3amalloc ((malloc_funcptr_t) TOPTR(0x00541A65))
#define pal3afree ((free_funcptr_t) TOPTR(0x005404C9))
#define pal3afsopen ((FILE *(*)(const char *, const char *, int)) TOPTR(0x00541DDE))
#define pal3afflush ((int (*)(FILE *)) TOPTR(0x00542ACD))
#define pal3afclose ((int (*)(FILE *)) TOPTR(0x00541BA4))
#define PrepareDir ((int (*)(void)) TOPTR(0x00523059))
#define PAL3_InitGFX ((void (*)(void)) TOPTR(0x00406F01))
#define PAL3_Create ((void (*)(HINSTANCE)) TOPTR(0x0040673A))
#define PAL3_Destroy ((void (*)(void)) TOPTR(0x00407C9D))
#define PAL3_Update ((void (*)(double)) TOPTR(0x004073B0))
#define gbBinkVideo_SFLB_OpenFile(this, szFileName, hWnd, bChangeScreenMode, nOpenFlag) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x005254C0, int, struct gbBinkVideo *, const char *, HWND, int, int), this, szFileName, hWnd, bChangeScreenMode, nOpenFlag)
#define gbBinkVideo_DoModal(this, bCanSkip) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00525382, int, struct gbBinkVideo *, int), this, bCanSkip)
#define WndProc ((LRESULT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM)) TOPTR(0x00406C91))
#define UICursor_Inst ((struct UICursor *(*)(void)) TOPTR(0x0052B734))
#define UICursor_Show(this, bShow) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0052B800, void, struct UICursor *, bool), this, bShow)
#define GRPinput_AcquireMouse(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00402B25, void, struct GRPinput *this), this)
#define GRPinput_AcquireKeyboard(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00402B78, void, struct GRPinput *this), this)
#define CCBRoleState_IsAlive(this, nIndex) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004DEFFD, bool, struct CCBRoleState *, int), this, nIndex)
#define BinkDoFrame (*(int (__stdcall **)(HBINK)) 0x005581DC)
#define BinkCopyToBuffer (*(int (__stdcall **)(HBINK, void *, int, unsigned, unsigned, unsigned, unsigned)) 0x005581D4)
#define BinkSetVolume (*(void (__stdcall **)(HBINK, int)) 0x005581D8)
#define g_bink (*(struct gbBinkVideo *) TOPTR(0x00A3E898))
#define SoundMgr_Inst() ((struct SoundMgr *) TOPTR(0x021AE020))
#define SoundMgr_GetAudioMgr(this) ((this)->m_audiodrv)
#define gbAudioManager_GetMusicMasterVolume(this) ((this)->MusicMasterVol)
#define gbBinkVideo_Width(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x005254A9, int, struct gbBinkVideo *), this)
#define gbBinkVideo_Height(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x005254B4, int, struct gbBinkVideo *), this)
#define gbBinkVideo_DrawFrameEx(this, pDestBuf, nDestPitch, nDestHeight, nDestLeft, nDestTop, nDestSurfaceType) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0052532F, int, struct gbBinkVideo *, void *, int, int, int, int, int), this, pDestBuf, nDestPitch, nDestHeight, nDestLeft, nDestTop, nDestSurfaceType)
#define gbBinkVideo_BinkWait(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00525291, int, struct gbBinkVideo *), this)
#define CTrail_Begin(this, pCam) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004B7424, void, struct CTrail *, struct gbCamera *), this, pCam)
#define UICursor_IRender(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0052B769, void, struct UICursor *), this)
#define RenderTarget_Inst ((struct RenderTarget *(*)(void)) TOPTR(0x004ADCF5))
#define UIDrawTextEx ((void (*)(const char *, RECT *, struct gbPrintFont *, int, int)) TOPTR(0x0052A73C))
#define UIPrint ((void (*)(int, int, char *, struct gbColorQuad *, int)) TOPTR(0x0052A54C))
#define UI3DCtrl_SetOriginPt_XY(this, x, y) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00441E73, void, struct UI3DCtrl *, int, int), this, x, y)
#define UI3DCtrl_SetOriginPt_XYFromY(this, x, y, from_y) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00441F34, void, struct UI3DCtrl *, int, int, int), this, x, y, from_y)
#define UI3DCtrl_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00441ACA, void, struct UI3DCtrl *), this)
#define UI3DCtrl_Update(this, deltatime, haveinput) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00441FFA, int, struct UI3DCtrl *, float, int), this, deltatime, haveinput)
#define UIStaticEX_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044B319, void, struct UIStaticEX *), this)
#define UIStaticEXA_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044B611, void, struct UIStaticEXA *), this)
#define UnderWater_Inst ((struct UnderWater *(*)(void)) TOPTR(0x004AFA9E))
#define C2DSpark_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004CA00F, void, struct C2DSpark *), this)
#define C2DSpark_CreateSingle(this, pSpark) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004C9D01, bool, struct C2DSpark *, struct C2DSpark_tagSpark *), this, pSpark)
#define C2DSpark_CreateStars(this, x, y, nWidth, fStarSize) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004C9E96, void, struct C2DSpark *, int, int, int, float), this, x, y, nWidth, fStarSize)
#define C2DSpark_CreateMore(this, x, y, color) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004C9D66, void, struct C2DSpark *, int, int, struct gbColorQuad), this, x, y, color)
#define CCBUI_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0050B5F6, bool, struct CCBUI *), this)
#define UIWnd_MoveWindow(this, x, y) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044D075, void, struct UIWnd *, int, int), this, x, y)
#define UIFrameWnd_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00445CF3, void, struct UIFrameWnd *), this)
#define CCBUI_GetNimbusArea(this, rc, nimbustype) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0050E7EC, RECT *, struct CCBUI *, RECT *, enum ECBFiveNimbus), this, rc, nimbustype)
#define ObjectCamera_Create(this, pMgr) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0047B079, void, struct ObjectCamera *, struct gbGfxManager *), this, pMgr)
#define LineupUI_Create(this, pWnd) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00480EBF, void, struct LineupUI *, struct UIWnd *), this, pWnd)
#define LineupComp_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x005215DE, void, struct LineupComp *), this)
#define UICoverFrm_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00451581, void, struct UICoverFrm *), this)
#define ArtistPlugIn_GetPlugFileInfo(this, filename) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044789A, void, struct ArtistPlugIn *, const char *), this, filename)
#define HeadMsg_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0040352F, void, struct HeadMsg *), this)
#define PlayerMgr_DrawMsg ((void (*)(void)) TOPTR(0x0040C22C))
#define UIAnimateCtrl_Render(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00442733, void, struct UIAnimateCtrl *), this)
#define UIWnd_Render_rewrited(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044CFE8, void, struct UIWnd *), this)
#define UIGameFrm_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00455751, void, struct UIGameFrm *), this)
#define UIGameOver_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00456524, void, struct UIGameOver *), this)
#define UIRoleDialog_Create(this, id, rect, pfather, bkfile) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x004571FB, void, struct UIRoleDialog *, int, RECT *, struct UIWnd *, const char *), this, id, rect, pfather, bkfile)
#define UIRoleDialog_SetFace(this, path, leftright) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00457481, void, struct UIRoleDialog *, const char *, int), this, path, leftright)
#define UIBigMap_Create(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0044EA6B, void, struct UIBigMap *), this)
#define CG_Entry_GetCGEntry ((struct CG_Entry *(*)(void)) TOPTR(0x0040779C))
#define PropUI_Create(this, pWnd) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0046F512, void, struct PropUI *, struct UIWnd *), this, pWnd)
#define MUIDialog_DoModel(this, havesnd) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x0049B948, void, struct MUIDialog *, bool), this, havesnd)
#define Archive_Save(this, index) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00523213, BOOL, struct Archive *, int), this, index)
#define Archive_Load(this, index) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00522A3F, BOOL, struct Archive *, int), this, index)
#define UpdateLoading ((void (*)(void)) TOPTR(0x0041E824))
#define gbCreateGraphManager ((struct gbGfxManager *(*)(struct gbGfxDriverInfo *)) TOPTR(gboffset + 0x1001DBF0))
#define UIDragBar_SetMaxVal(this, val) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00444528, void, struct UIDragBar *, float), this, val)



// global variables
#define GB_GfxMgr (*(struct gbGfxManager_D3D **) TOPTR(0x00C01CD4))
#define PAL3_s_drvinfo (*(struct gbGfxDriverInfo *) TOPTR(0x00C01788))
#define PAL3_s_flag (*(unsigned *) TOPTR(0x00574D38))
#define PAL3_s_gamestate (*(int *) TOPTR(0x00C01CE0))
#define PAL3_s_bActive (*(int *) TOPTR(0x00574D34))
#define PAL3_m_screenImg (*(struct gbImage2D *) TOPTR(0x008AC168))
#define xmusic ((PAL3_s_flag & 4) == 0)
#define ui_tex_color_gbf (*(struct gbRenderEffect **) TOPTR(0x0228F004))
#define ui_color_blend_tex_gbf (*(struct gbRenderEffect **) TOPTR(0x0228F01C))
#define g_pVFileSys (*(struct gbVFileSystem **) TOPTR(gboffset + 0x10131D08))
#define g_msgbk (*(struct UIStaticFlex *) TOPTR(0x00C01D50))
#define g_gamefrm (*(struct UIGameFrm *) TOPTR(0x00F79E98))
#define g_input (*(struct GRPinput *) TOPTR(0x00BFEE30))
#define g_lastupdateloading (*(DWORD *) TOPTR(0x00C1F70C))
#define PlayerMgr_m_RoleTask (*(struct RoleTaskPool *) TOPTR(0x00C10E78))


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
    assert(sizeof(struct gbRefObject) == 0x8); \
    assert(sizeof(struct gbResource) == 0x18); \
    assert(sizeof(struct gbResManager) == 0xC); \
    assert(sizeof(struct CD3DSettings) == 0x6C); \
    assert(sizeof(struct gbGfxDriverInfo) == 0xB8); \
    assert(sizeof(struct gbGfxManager) == 0x5F0); \
    assert(sizeof(struct gbGfxManager_D3D) == 0x8F0); \
    assert(sizeof(struct gbUIQuad) == 0x28); \
    assert(sizeof(struct CPKHeader) == 0x80); \
    assert(sizeof(struct CPKTable) == 0x1C); \
    assert(sizeof(struct CPK) == 0xE01BC); \
    assert(sizeof(struct gbBinkVideo) == 0x1C0388); \
    assert(sizeof(struct D3DAdapterInfo) == 0x458); \
    assert(sizeof(struct UICursor) == 0x20); \
    assert(sizeof(struct gbCamera) == 0x178); \
    assert(sizeof(struct gbQuaternion) == 0x10); \
    assert(sizeof(struct GRP_KEYREG) == 0x10); \
    assert(sizeof(struct GRPinput) == 0x2958); \
    assert(sizeof(struct tagAttackSequen) == 0x28); \
    assert(sizeof(struct CCBAttackSequen) == 0x1F4); \
    assert(sizeof(struct tagCmdData) == 0x2C); \
    assert(sizeof(struct tagThread) == 0x58); \
    assert(sizeof(struct tagPlayerSet) == 0x34); \
    assert(sizeof(struct CCBSystem) == 0x162D8); \
    assert(sizeof(struct SoundMgr) == 0xE050C); \
    assert(sizeof(struct TxtFile) == 0x20C); \
    assert(sizeof(struct gbAudioManager) == 0xF4); \
    assert(sizeof(struct SoundAttachObj) == 0xC); \
    assert(sizeof(struct gbSurfaceDesc) == 0x14); \
    assert(sizeof(struct gbPixelFormat) == 0x2C); \
    assert(sizeof(struct gbTexture) == 0x54); \
    assert(sizeof(struct gbTexture_D3D) == 0x60); \
    assert(sizeof(struct CTrail) == 0x348); \
    assert(sizeof(struct gbTextureArray) == 0x44); \
    assert(sizeof(struct gbDynVertBuf) == 0x3C); \
    assert(sizeof(struct gbPrintFont) == 0x4C); \
    assert(sizeof(struct RenderTarget) == 0xC8); \
    assert(sizeof(struct gbCachedChinaFont) == 0xC4); \
    assert(sizeof(struct UI3DObj) == 0x20); \
    assert(sizeof(struct UI3DCtrl) == 0x134); \
    assert(sizeof(struct UIStatic) == 0xA0); \
    assert(sizeof(struct UIStaticEX) == 0xC0); \
    assert(sizeof(struct UIStaticEXA) == 0xC8); \
    assert(sizeof(struct UnderWater) == 0x18); \
    assert(sizeof(struct C2DSpark_tagSpark) == 0x30); \
    assert(sizeof(struct tagShowItem) == 0x118); \
    assert(sizeof(struct UIFrameWnd) == 0x48); \
    assert(sizeof(struct tagUIParam) == 0x20); \
    assert(sizeof(struct tagUITask) == 0x48); \
    assert(sizeof(struct CCBUI) == 0xDE0); \
    assert(sizeof(struct CCBLineupWindow) == 0x7C); \
    assert(sizeof(struct tagImbibeNimbus) == 0x2C); \
    assert(sizeof(struct ObjectCamera) == 0x7C); \
    assert(sizeof(struct UICoverFrm) == 0x3AE8); \
    assert(sizeof(struct UIPar_Star) == 0x7C); \
    assert(sizeof(struct UIPar_Meteor) == 0x114); \
    assert(sizeof(struct _CLOUD) == 0xD8); \
    assert(sizeof(struct UITextureArray) == 0x44); \
    assert(sizeof(struct UIButton) == 0x158); \
    assert(sizeof(struct _btnOPTDATA) == 0x10); \
    assert(sizeof(struct UIButtonEX) == 0x188); \
    assert(sizeof(struct Average) == 0x18); \
    assert(sizeof(struct UIStaticFlex) == 0xC0); \
    assert(sizeof(struct UIDialog) == 0x48); \
    assert(sizeof(struct _TimeMgr) == 0xC); \
    assert(sizeof(struct MUIDialog) == 0x568); \
    assert(sizeof(struct gbImage2DInfo) == 0x80); \
    assert(sizeof(struct gbImage2D) == 0xC8); \
    assert(sizeof(struct gbVFileSystem) == 0xE01D4); \
    assert(sizeof(struct UIPosMaker) == 0xC); \
    assert(sizeof(struct ArtistPlugIn) == 0x230); \
    assert(sizeof(struct _Texture_uv) == 0x8); \
    assert(sizeof(struct _PlugInfo) == 0x248); \
    assert(sizeof(struct HeadMsg) == 0x44); \
    assert(sizeof(struct PalScriptWaitObj) == 0x8); \
    assert(sizeof(struct UICaption) == 0x84); \
    assert(sizeof(struct UINote) == 0x10C); \
    assert(sizeof(struct UIRoleSelect) == 0x620); \
    assert(sizeof(struct UIGameFrm) == 0x7A84); \
    assert(sizeof(struct UIFlexBar) == 0x54); \
    assert(sizeof(struct UIScrollBar) == 0x170); \
    assert(sizeof(struct UILBColumn) == 0x5C); \
    assert(sizeof(struct UIListBox) == 0x508); \
    assert(sizeof(struct UIHeadDialog) == 0x160); \
    assert(sizeof(struct UIRoleDialog) == 0x2D0); \
    assert(sizeof(struct UIAnimateCtrl) == 0xA4); \
    assert(sizeof(struct UIStaticFlip) == 0xA4); \
    assert(sizeof(struct UIChatRest) == 0x2660); \
    assert(sizeof(struct UITextArea) == 0x68); \
    assert(sizeof(struct UITabBtn) == 0x18); \
    assert(sizeof(struct TimeCtl) == 0xC); \
    assert(sizeof(struct RestScript) == 0x814); \
    assert(sizeof(struct ScriptData) == 0x508); \
    assert(sizeof(struct UIEmote) == 0x988); \
    assert(sizeof(struct UIProgressBar) == 0xF4); \
    assert(sizeof(struct UISceneMap) == 0x4B8); \
    assert(sizeof(struct UISceneFace) == 0x734); \
    assert(sizeof(struct UISceneFrm) == 0xC34); \
    assert(sizeof(struct UIGameOver) == 0xF0); \
    assert(sizeof(struct UIButtonEXA) == 0x178); \
    assert(sizeof(struct _MapElement) == 0x188); \
    assert(sizeof(struct UIBigMap) == 0x2860); \
    assert(sizeof(struct LEVELPARAM) == 0x8); \
    assert(sizeof(struct CG_Entry) == 0xCC); \
    assert(sizeof(struct UIStaticRM) == 0xBC); \
    assert(sizeof(struct st_Manager) == 0x4460); \
    assert(sizeof(struct CG_UI) == 0x9A50); \
    assert(sizeof(struct BtnGroup) == 0x2E4); \
    assert(sizeof(struct PropUI) == 0x72C8); \
    assert(sizeof(struct RoleTaskPool) == 0x2AC); \
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
