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
    union {
        struct {
            void *fp;
            BYTE gap4[12];
        };
        struct {
            unsigned long long gap0[2]; // force align
        };
    };
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



// PAL3A functions
#define pal3amalloc ((malloc_funcptr_t) TOPTR(0x00541A65))
#define pal3afree ((free_funcptr_t) TOPTR(0x005404C9))
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



// global variables
#define GB_GfxMgr (*(struct gbGfxManager_D3D **) TOPTR(0x00C01CD4))
#define PAL3_s_drvinfo (*(struct gbGfxDriverInfo *) TOPTR(0x00C01788))
#define PAL3_s_flag (*(unsigned *) TOPTR(0x00574D38))
#define PAL3_s_gamestate (*(int *) TOPTR(0x00C01CE0))
#define PAL3_s_bActive (*(int *) TOPTR(0x00574D34))
#define xmusic ((PAL3_s_flag & 4) == 0)


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
