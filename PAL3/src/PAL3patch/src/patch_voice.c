#include "common.h"

// avoid macro conflicts (dirty)
#undef BinkDoFrame
#undef BinkCopyToBuffer
#undef BinkSetVolume


// ABI version
#define VOICEPLUGIN_ABI_VERSION 2


// toolkit definition

struct MiscToolkit {
    // error handling functions
    void (WINAPI *ReportFatalError)(const char *msg);
    
    // hash functions
    unsigned int (WINAPI *CalcStringGBCRC32)(const char *str);
    void (WINAPI *CalcStringSHA1)(char *out_str, const char *str);
    
    // rect functions
    void (WINAPI *GetRatioRect)(RECT *out_rect, const RECT *rect, double ratio);
};

struct GraphicsToolkit {
    HWND hWnd;
    int Width;
    int Height;
    IDirect3D9 *pD3D;
    IDirect3DDevice9 *pd3dDevice;
    void (WINAPI *EnsureCooperativeLevel)(void);
    void (WINAPI *(WINAPI *SetOnLostDeviceCallback)(void (WINAPI *)(void)))(void);
    void (WINAPI *(WINAPI *SetOnResetDeviceCallback)(void (WINAPI *)(void)))(void);
};

struct CursorToolkit {
    void (WINAPI *RenderSoftCursor)(void);
    int (WINAPI *GetShowCursorState)(void);
    void (WINAPI *SetShowCursorState)(int show);
};

struct VolumeToolkit {
    // get and set background music volume
    float (WINAPI *GetMusicMasterVolume)(void);
    void (WINAPI *SetMusicMasterVolume)(float volume);
    float (WINAPI *Get2DMasterVolume)(void);
    void (WINAPI *Set2DMasterVolume)(float volume);
    float (WINAPI *Get3DMasterVolume)(void);
    void (WINAPI *Set3DMasterVolume)(float volume);
};

struct BinkToolkit {
    HBINK *pMovieHandle; // currently opened movie
    
    // bink functions
    HBINK (WINAPI *BinkOpen)(const char *file_name, unsigned open_flags);
    int (WINAPI *BinkDoFrame)(HBINK bink);
    void (WINAPI *BinkNextFrame)(HBINK bink);
    int (WINAPI *BinkWait)(HBINK bink);
    int (WINAPI *BinkCopyToBuffer)(HBINK bink, void *dest_addr, int dest_pitch, unsigned dest_height, unsigned dest_x, unsigned dest_y, unsigned copy_flags);
    void (WINAPI *BinkClose)(HBINK bink);
    void (WINAPI *BinkSetVolume)(HBINK bink, int volume);
    int (WINAPI *BinkPause)(HBINK bink, int pause);
};

struct MSSToolkit {
    HDIGDRIVER h2DDriver;
    
    // mss32 functions
    HSTREAM (WINAPI *AIL_open_stream)(HDIGDRIVER dig, const char *name, int stream_mem);
    void (WINAPI *AIL_start_stream)(HSTREAM stream);
    void (WINAPI *AIL_close_stream)(HSTREAM stream);
    void (WINAPI *AIL_pause_stream)(HSTREAM stream, int onoff);
    void (WINAPI *AIL_set_stream_volume)(HSTREAM stream, int volume);
    void (WINAPI *AIL_set_stream_loop_count)(HSTREAM stream, int loop_count);
    int (WINAPI *AIL_stream_status)(HSTREAM stream);
    AILSTREAMCB (WINAPI *AIL_register_stream_callback)(HSTREAM stream, AILSTREAMCB callback);
    void (WINAPI *AIL_stream_ms_position)(HSTREAM stream, int *total_milliseconds, int *current_milliseconds);
    void (WINAPI *AIL_set_stream_ms_position)(HSTREAM stream, int milliseconds);
};

struct VoiceToolkit {
    // patch version
    const char *patch_version;
    const char *build_date;

    struct MiscToolkit *misc;
    struct GraphicsToolkit *gfx;
    struct CursorToolkit *curs;
    struct VolumeToolkit *vol;
    struct BinkToolkit *bik;
    struct MSSToolkit *mss;
};








// imported function pointers

//   plugin management functions
static int (WINAPI *VoiceDLLAttached)(void);
static void (WINAPI *VoiceInit)(struct VoiceToolkit *toolkit);
static void (WINAPI *VoiceCleanup)(void);
static void (WINAPI *GamePause)(void);
static void (WINAPI *GameResume)(void);
//   dialog text functions
static void (WINAPI *TextIdle)(int state);
static void (WINAPI *TextPrepare)(const char *text, int mode);
static void (WINAPI *TextStart)(void);
static void (WINAPI *TextStop)(void);
//   caption functions
static void (WINAPI *CaptionIdle)(int state);
static void (WINAPI *CaptionPrepare)(const char *tex);
static void (WINAPI *CaptionStart)(void);
static void (WINAPI *CaptionStop)(void);
//   movie functions
static void (WINAPI *MovieIdle)(int state);
static void (WINAPI *MoviePrepare)(const char *movie);
static void (WINAPI *MovieStart)(void);
static void (WINAPI *MovieStop)(void);
//   combat dialog functions
static void (WINAPI *CBDialogIdle)(int state);
static void (WINAPI *CBDialogPrepare)(const char *text);
static void (WINAPI *CBDialogStart)(void);
static void (WINAPI *CBDialogStop)(void);












// toolkit functions
static struct gbAudioManager *pAudioMgr = NULL;

static void WINAPI ReportFatalError(const char *msg)
{
    fail("voice plugin fatal error: %s", msg ? msg : "no message");
}

static unsigned WINAPI CalcStringGBCRC32(const char *str)
{
    return gbCrc32Compute(str);
}

static void WINAPI CalcStringSHA1(char *out_str, const char *str)
{
    // out_str should at least SHA1_STR_SIZE bytes
    unsigned char sha1buf[SHA1_BYTE];
    sha1_hash_buffer(str, strlen(str), sha1buf);
    strcpy(out_str, sha1_tostr(sha1buf));
}

static void WINAPI GetRatioRect(RECT *out_rect, const RECT *rect, double ratio)
{
    fRECT frect;
    set_frect_rect(&frect, rect);
    get_ratio_frect(&frect, &frect, ratio, TR_CENTER, TR_CENTER);
    set_rect_frect(out_rect, &frect);
}

static void WINAPI EnsureCooperativeLevel(void)
{
    gbGfxManager_D3D_EnsureCooperativeLevel(GB_GfxMgr, 1);
};

static void (WINAPI *OnLostDeviceCallback)(void) = NULL;
static void (WINAPI *OnResetDeviceCallback)(void) = NULL;
static void (WINAPI *(WINAPI SetOnLostDeviceCallback)(void (WINAPI *fp)(void)))(void)
{
    void (WINAPI *ret)(void) = fp;
    OnLostDeviceCallback = fp;
    return ret;
}
static void (WINAPI *(WINAPI SetOnResetDeviceCallback)(void (WINAPI *fp)(void)))(void)
{
    void (WINAPI *ret)(void) = fp;
    OnResetDeviceCallback = fp;
    return ret;
}
static void WINAPI RenderSoftCursor(void)
{
    // force vertex buffer update in gbDynVertBuf_D3D::End()
    GB_GfxMgr->m_pCacheVB = NULL;
    
    // render cursor
    render_softcursor();
}
static int WINAPI GetShowCursorState(void)
{
    return get_showcursor_state();
}
static void WINAPI SetShowCursorState(int show)
{
    set_showcursor_state(show);
}
static float WINAPI GetMusicMasterVolume()
{
    return pAudioMgr && !xmusic ? gbAudioManager_GetMusicMasterVolume(pAudioMgr) : 0.0f;
}
static void WINAPI SetMusicMasterVolume(float volume)
{
    if (pAudioMgr && !xmusic) gbAudioManager_SetMusicMasterVolume(pAudioMgr, volume);
}
static float WINAPI Get2DMasterVolume()
{
    return pAudioMgr && !xsnd ? gbAudioManager_Get2DMasterVolume(pAudioMgr) : 0.0f;
}
static void WINAPI Set2DMasterVolume(float volume)
{
    if (pAudioMgr && !xsnd) gbAudioManager_Set2DMasterVolume(pAudioMgr, volume);
}
static float WINAPI Get3DMasterVolume()
{
    return pAudioMgr && !xsnd ? gbAudioManager_Get3DMasterVolume(pAudioMgr) : 0.0f;
}
static void WINAPI Set3DMasterVolume(float volume)
{
    if (pAudioMgr && !xsnd) gbAudioManager_Set3DMasterVolume(pAudioMgr, volume);
}






// voice plugin flags
static int plugin_init_flag = 0;





//  dialog

//
//  Prepare() should be idempotent
//
//        Prepare()
//      *<<<<<<<<<*
//      v         | Prepare()            Start()
//    CLOSED  >>>>*>>>>>>>>>>  OPENING  >>>>>>>> OPENED
//      ^                                          |
//      *<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*
//                         End()
enum {
    TEXT_CLOSED,  // End() -> Prepare()
    TEXT_OPENING, // Prepare() -> Start()
    TEXT_OPENED,  // Start() -> End()
};

static int roledlg_state = TEXT_CLOSED;

static void try_prepare(const char *text, int mode)
{
    if (text) {
        switch (roledlg_state) {
            case TEXT_OPENED:
                TextStop();
            case TEXT_CLOSED:
            case TEXT_OPENING:
                TextPrepare(text, mode);
                roledlg_state = TEXT_OPENING;
                break;
        }
    }
}

static void try_start()
{
    switch (roledlg_state) {
        case TEXT_OPENED:
            TextStop();
        case TEXT_CLOSED:
            roledlg_state = TEXT_CLOSED;
            break;
        case TEXT_OPENING:
            TextStart();
            roledlg_state = TEXT_OPENED;
            break;
    }
}

static void try_end()
{
    switch (roledlg_state) {
        case TEXT_OPENING:
        case TEXT_CLOSED:
            roledlg_state = TEXT_CLOSED;
            break;
        case TEXT_OPENED:
            TextStop();
            roledlg_state = TEXT_CLOSED;
            break;
    }
}

static void try_idle(int is_opened)
{
    if (is_opened) {
        switch (roledlg_state) {
            case TEXT_CLOSED:
                roledlg_state = TEXT_CLOSED;
                break;
            case TEXT_OPENING:
                TextStart();
            case TEXT_OPENED:
                roledlg_state = TEXT_OPENED;
                break;
        }
    } else {
        switch (roledlg_state) {
            case TEXT_OPENING:
                roledlg_state = TEXT_OPENING;
                break;
            case TEXT_CLOSED:
                roledlg_state = TEXT_CLOSED;
                break;
            case TEXT_OPENED:
                TextStop();
                roledlg_state = TEXT_CLOSED;
                break;
        }
    }
    TextIdle(roledlg_state);
}

struct txt_state_t {
    int visible;
    int bkstate;
};

static struct txt_state_t last_txtstate;
static struct txt_state_t cur_txtstate = {
    .visible = 0,
    .bkstate = UISTATICFLEX_OPENING,
};

static void text_gameloop_proc()
{
    // check if created
    if (!pUIWND(&g_gamefrm.m_roledlg)->m_bcreateok) return;
    
    // roll state
    last_txtstate = cur_txtstate;
    
    // dump current state
    cur_txtstate = (struct txt_state_t) {
        .visible = pUIWND(&g_gamefrm.m_roledlg)->m_bvisible,
        .bkstate = g_gamefrm.m_roledlg.m_bk.m_state,
    };
    
    // check status
    if (!last_txtstate.visible && cur_txtstate.visible) {
        try_prepare(g_gamefrm.m_roledlg.m_static.m_buf._Ptr, g_gamefrm.m_roledlg.m_mode);
    } else if (last_txtstate.bkstate != UISTATICFLEX_NORMAL && cur_txtstate.bkstate == UISTATICFLEX_NORMAL) {
        try_start();
    } else if (last_txtstate.visible && !cur_txtstate.visible) {
        try_end();
    } else {
        try_idle(cur_txtstate.visible && cur_txtstate.bkstate == UISTATICFLEX_NORMAL);
    }
}



//   caption

static int last_capstate, cur_capstate = 0;
static void caption_gameloop_proc()
{
    // check if created
    if (!pUIWND(&g_gamefrm.m_cap)->m_bcreateok) return;
    
    // roll state
    last_capstate = cur_capstate;
    
    // dump state
    cur_capstate = pUIWND(&g_gamefrm.m_cap)->m_bvisible;
    
    // check status
    if (cur_capstate && !last_capstate) {
        CaptionPrepare(g_gamefrm.m_cap.m_tex->baseclass.pName);
        CaptionStart();
    } else if (!cur_capstate && last_capstate) {
        CaptionStop();
    } else {
        CaptionIdle(!!cur_capstate);
    }
}




// movie
static void movie_gameloop_proc(struct game_loop_hook_data *hookarg)
{
    switch (hookarg->type) {
        case GAMELOOP_NORMAL:
            MovieIdle(0);
            break;
        case GAMELOOP_MOVIE:
            MovieIdle(1);
            break;
        case GAMEEVENT_MOVIE_ATOPEN:
            MoviePrepare(hookarg->data);
            break;
        case GAMEEVENT_MOVIE_ATBEGIN:
            MovieStart();
            break;
        case GAMEEVENT_MOVIE_ATEND:
            MovieStop();
            break;
        default: break;
    }
}







// combat dialog
static int cbdialog_started = 0;
static int cbdialog_lastvisible = 0;
static char *cbdialog_laststr = NULL;
static void cbdialog_gameloop_proc()
{
    if (!PAL3_m_pCBSystem || !PAL3_m_pCBSystem->m_pUI || !PAL3_m_pCBSystem->m_pUI->m_pDialogBack || !pUIWND(PAL3_m_pCBSystem->m_pUI->m_pDialogBack)->m_bcreateok) return;
    
    struct UIStatic *cbd = PAL3_m_pCBSystem->m_pUI->m_pDialogBack;

    int curvisible = cbd->baseclass.m_bvisible;
    char *curstr = cbd->m_text._Ptr;
    
    if ((!cbdialog_lastvisible && curvisible) || 
        (curvisible && cbdialog_laststr && curstr && strcmp(cbdialog_laststr, curstr) != 0)) {
        if (cbdialog_started) {
            CBDialogStop();
            cbdialog_started = 0;
        }
        if (curstr) {
            CBDialogPrepare(curstr);
            CBDialogStart();
            cbdialog_started = 1;
        }
    } else if (cbdialog_lastvisible && !curvisible) {
        if (cbdialog_started) {
            CBDialogStop();
            cbdialog_started = 0;
        }
    } else {
        CBDialogIdle(!!curvisible);
    }
    
    cbdialog_lastvisible = curvisible;
    free(cbdialog_laststr);
    cbdialog_laststr = curstr ? strdup(curstr) : NULL;
}







// d3d lost and reset device

static void voice_onlostdevice_hook()
{
    if (OnLostDeviceCallback) OnLostDeviceCallback();
}

static void voice_onresetdevice_hook()
{
    if (OnResetDeviceCallback) OnResetDeviceCallback();
}




// init and cleanup

static void voice_prepal3destroy_hook()
{
    VoiceCleanup();
    plugin_init_flag = 0;
}

static void voice_postgamecreate_hook()
{
    // allocate static memory
    static struct VoiceToolkit toolkit;
    static struct MiscToolkit misc;
    static struct GraphicsToolkit gfx;
    static struct CursorToolkit curs;
    static struct VolumeToolkit vol;
    static struct BinkToolkit bik;
    static struct MSSToolkit mss;
    
    // get AudioManager
    pAudioMgr = SoundMgr_GetAudioMgr(SoundMgr_Inst());
    
    // set toolkit
    
    toolkit = (struct VoiceToolkit) {
        .patch_version = patch_version,
        .build_date = build_date,
        
        .misc = &misc,
        .gfx = &gfx,
        .curs = &curs,
        .vol = &vol,
        .bik = &bik,
        .mss = &mss,
    };
    
    misc = (struct MiscToolkit) {
        .ReportFatalError = ReportFatalError,
        .CalcStringGBCRC32 = CalcStringGBCRC32,
        .CalcStringSHA1 = CalcStringSHA1,
        .GetRatioRect = GetRatioRect,
    };
    
    gfx = (struct GraphicsToolkit) {
        .hWnd = PAL3_s_drvinfo.hgfxwnd,
        .Width = PAL3_s_drvinfo.width,
        .Height = PAL3_s_drvinfo.height,
        .pD3D = GB_GfxMgr->m_pD3D,
        .pd3dDevice = GB_GfxMgr->m_pd3dDevice,
        .EnsureCooperativeLevel = EnsureCooperativeLevel,
        .SetOnLostDeviceCallback = SetOnLostDeviceCallback,
        .SetOnResetDeviceCallback = SetOnResetDeviceCallback,
    };
    
    curs = (struct CursorToolkit) {
        .RenderSoftCursor = RenderSoftCursor,
        .GetShowCursorState = GetShowCursorState,
        .SetShowCursorState = SetShowCursorState,
    };
        
    vol = (struct VolumeToolkit) {
        .GetMusicMasterVolume = GetMusicMasterVolume,
        .SetMusicMasterVolume = SetMusicMasterVolume,
        .Get2DMasterVolume = Get2DMasterVolume,
        .Set2DMasterVolume = Set2DMasterVolume,
        .Get3DMasterVolume = Get3DMasterVolume,
        .Set3DMasterVolume = Set3DMasterVolume,
    };
    
    HMODULE binkw32 = GetModuleHandle_check("BINKW32.DLL");
    bik = (struct BinkToolkit) {
        .pMovieHandle = &g_bink.m_hBink,
        .BinkOpen = TOPTR(GetProcAddress_check(binkw32, "_BinkOpen@8")),
        .BinkDoFrame = TOPTR(GetProcAddress_check(binkw32, "_BinkDoFrame@4")),
        .BinkNextFrame = TOPTR(GetProcAddress_check(binkw32, "_BinkNextFrame@4")),
        .BinkWait = TOPTR(GetProcAddress_check(binkw32, "_BinkWait@4")),
        .BinkCopyToBuffer = TOPTR(GetProcAddress_check(binkw32, "_BinkCopyToBuffer@28")),
        .BinkClose = TOPTR(GetProcAddress_check(binkw32, "_BinkClose@4")),
        .BinkSetVolume = TOPTR(GetProcAddress_check(binkw32, "_BinkSetVolume@8")),
        .BinkPause = TOPTR(GetProcAddress_check(binkw32, "_BinkPause@8")),
    };
    
    HMODULE mss32 = GetModuleHandle_check("MSS32.DLL");
    mss = (struct MSSToolkit) {
        .h2DDriver = pAudioMgr ? pAudioMgr->h2DDriver : NULL,
        .AIL_open_stream = TOPTR(GetProcAddress_check(mss32, "_AIL_open_stream@12")),
        .AIL_start_stream = TOPTR(GetProcAddress_check(mss32, "_AIL_start_stream@4")),
        .AIL_close_stream = TOPTR(GetProcAddress_check(mss32, "_AIL_close_stream@4")),
        .AIL_pause_stream = TOPTR(GetProcAddress_check(mss32, "_AIL_pause_stream@8")),
        .AIL_set_stream_volume = TOPTR(GetProcAddress_check(mss32, "_AIL_set_stream_volume@8")),
        .AIL_set_stream_loop_count = TOPTR(GetProcAddress_check(mss32, "_AIL_set_stream_loop_count@8")),
        .AIL_stream_status = TOPTR(GetProcAddress_check(mss32, "_AIL_stream_status@4")),
        .AIL_register_stream_callback = TOPTR(GetProcAddress_check(mss32, "_AIL_register_stream_callback@8")),
        .AIL_stream_ms_position = TOPTR(GetProcAddress_check(mss32, "_AIL_stream_ms_position@12")),
        .AIL_set_stream_ms_position = TOPTR(GetProcAddress_check(mss32, "_AIL_set_stream_ms_position@8")),
    };
    
    // init voice plugin
    VoiceInit(&toolkit);
    
    plugin_init_flag = 1;
}



// game pause and resume

static void voice_checkpause_hook(void *arg)
{
    int paused = *(int *) arg;
    if (paused) {
        GamePause();
    } else {
        GameResume();
    }
}




// gameloop dispatcher

static void voice_gameloop_dispatcher(void *arg)
{
    if (!plugin_init_flag) return;
    
    struct game_loop_hook_data *hookarg = arg;
    switch (hookarg->type) {
        case GAMELOOP_NORMAL:
            text_gameloop_proc();
            caption_gameloop_proc();
            movie_gameloop_proc(hookarg);
            cbdialog_gameloop_proc();
            break;
        case GAMELOOP_MOVIE:
        case GAMEEVENT_MOVIE_ATOPEN:
        case GAMEEVENT_MOVIE_ATBEGIN:
        case GAMEEVENT_MOVIE_ATEND:
            movie_gameloop_proc(hookarg);
            break;
        default: break;
    }
}



MAKE_PATCHSET(voice)
{
    // init hooks
    add_postgamecreate_hook(voice_postgamecreate_hook);
    add_prepal3destroy_hook(voice_prepal3destroy_hook);
    add_gameloop_hook(voice_gameloop_dispatcher);
    add_pauseresume_hook(voice_checkpause_hook);
    add_onlostdevice_hook(voice_onlostdevice_hook);
    add_onresetdevice_hook(voice_onresetdevice_hook);



    // dynlink voice plugin    
    HMODULE hPlugin = LoadLibraryW_check(cs2wcs(get_string_from_configfile("voiceplugin"), CP_UTF8));

    VoiceDLLAttached = TOPTR(GetProcAddress_check(hPlugin, "_VoiceDLLAttached@0"));
    int abiver = VoiceDLLAttached();
    if (abiver != VOICEPLUGIN_ABI_VERSION) {
        fail("voice plugin requires unsupported ABI version %d (only %d is supported in this patch).", abiver, VOICEPLUGIN_ABI_VERSION);
    }
    
    VoiceInit = TOPTR(GetProcAddress_check(hPlugin, "_VoiceInit@4"));
    VoiceCleanup = TOPTR(GetProcAddress_check(hPlugin, "_VoiceCleanup@0"));
    GamePause = TOPTR(GetProcAddress_check(hPlugin, "_GamePause@0"));
    GameResume = TOPTR(GetProcAddress_check(hPlugin, "_GameResume@0"));
    
    TextIdle = TOPTR(GetProcAddress_check(hPlugin, "_TextIdle@4"));
    TextPrepare = TOPTR(GetProcAddress_check(hPlugin, "_TextPrepare@8"));
    TextStart = TOPTR(GetProcAddress_check(hPlugin, "_TextStart@0"));
    TextStop = TOPTR(GetProcAddress_check(hPlugin, "_TextStop@0"));
    
    CaptionIdle = TOPTR(GetProcAddress_check(hPlugin, "_CaptionIdle@4"));
    CaptionPrepare = TOPTR(GetProcAddress_check(hPlugin, "_CaptionPrepare@4"));
    CaptionStart = TOPTR(GetProcAddress_check(hPlugin, "_CaptionStart@0"));
    CaptionStop = TOPTR(GetProcAddress_check(hPlugin, "_CaptionStop@0"));

    MovieIdle = TOPTR(GetProcAddress_check(hPlugin, "_MovieIdle@4"));
    MoviePrepare = TOPTR(GetProcAddress_check(hPlugin, "_MoviePrepare@4"));
    MovieStart = TOPTR(GetProcAddress_check(hPlugin, "_MovieStart@0"));
    MovieStop = TOPTR(GetProcAddress_check(hPlugin, "_MovieStop@0"));
    
    CBDialogIdle = TOPTR(GetProcAddress_check(hPlugin, "_CBDialogIdle@4"));
    CBDialogPrepare = TOPTR(GetProcAddress_check(hPlugin, "_CBDialogPrepare@4"));
    CBDialogStart = TOPTR(GetProcAddress_check(hPlugin, "_CBDialogStart@0"));
    CBDialogStop = TOPTR(GetProcAddress_check(hPlugin, "_CBDialogStop@0"));
}
