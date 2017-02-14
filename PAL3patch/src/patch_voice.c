#include "common.h"

// toolkit

struct VoiceToolkit {
    // patch version
    const char *patch_version;
    const char *build_date;
    
    // some information
    HWND hgfxwnd;
    HBINK *phbink;
    double *pcurtime;
    
    // error handling functions
    void (WINAPI *ReportFatalError)(const char *msg);
    
    // hash functions
    unsigned int (WINAPI *CalcStringCRC32)(const char *str);
    
    // get and set background music volume
    float (WINAPI *GetMusicMasterVolume)(void);
    void (WINAPI *SetMusicMasterVolume)(float volume);
    float (WINAPI *Get2DMasterVolume)(void);
    void (WINAPI *Set2DMasterVolume)(float volume);
    float (WINAPI *Get3DMasterVolume)(void);
    void (WINAPI *Set3DMasterVolume)(float volume);
    
    // mss32 functions
    HDIGDRIVER dig;
    HSTREAM (WINAPI *AIL_open_stream)(HDIGDRIVER dig, const char *name, int stream_mem);
    void (WINAPI *AIL_start_stream)(HSTREAM stream);
    void (WINAPI *AIL_close_stream)(HSTREAM stream);
    void (WINAPI *AIL_pause_stream)(HSTREAM stream, int onoff);
    void (WINAPI *AIL_set_stream_volume)(HSTREAM stream, int volume);
    void (WINAPI *AIL_set_stream_loop_count)(HSTREAM stream, int loop_count);
    int (WINAPI *AIL_stream_status)(HSTREAM stream);
    AILSTREAMCB (WINAPI *AIL_register_stream_callback)(HSTREAM stream, AILSTREAMCB callback);
    void (WINAPI *AIL_stream_ms_position)(HSTREAM stream, int *total_milliseconds, int *current_milliseconds);
    void (WINAPI *AIL_set_stream_ms_position)(HSTREAM hstream, int milliseconds);
};





// imported function pointers


// plugin management functions
static void (WINAPI *VoiceDLLAttached)(void);
static void (WINAPI *VoiceInit)(struct VoiceToolkit *toolkit);
static void (WINAPI *VoiceCleanup)(void);
static void (WINAPI *GamePause)(void);
static void (WINAPI *GameResume)(void);

// dialog text functions
static void (WINAPI *TextIdle)(int state);
static void (WINAPI *TextPrepare)(const char *text, int mode);
static void (WINAPI *TextStart)(void);
static void (WINAPI *TextStop)(void);

// caption functions
static void (WINAPI *CaptionIdle)(int state);
static void (WINAPI *CaptionPrepare)(const char *tex);
static void (WINAPI *CaptionStart)(void);
static void (WINAPI *CaptionStop)(void);

// movie functions
static void (WINAPI *MovieIdle)(int state);
static void (WINAPI *MoviePrepare)(const char *movie);
static void (WINAPI *MovieStart)(void);
static void (WINAPI *MovieStop)(void);








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

static void WINAPI ReportFatalError(const char *msg)
{
    fail("voice plugin fatal error: %s", msg ? msg : "no message");
}

static unsigned WINAPI CalcStringCRC32(const char *str)
{
    return gbCrc32Compute(str);
}

static float WINAPI GetMusicMasterVolume()
{
    return gbAudioManager_GetMusicMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()));
}
static void WINAPI SetMusicMasterVolume(float volume)
{
    gbAudioManager_SetMusicMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()), volume);
}
static float WINAPI Get2DMasterVolume()
{
    return gbAudioManager_Get2DMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()));
}
static void WINAPI Set2DMasterVolume(float volume)
{
    gbAudioManager_Set2DMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()), volume);
}
static float WINAPI Get3DMasterVolume()
{
    return gbAudioManager_Get3DMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()));
}
static void WINAPI Set3DMasterVolume(float volume)
{
    gbAudioManager_Set3DMasterVolume(SoundMgr_GetAudioMgr(SoundMgr_Inst()), volume);
}

struct dlg_state_t {
    int visible;
    int bkstate;
};

static struct dlg_state_t last_dlgstate;
static struct dlg_state_t cur_dlgstate = {
    .visible = 0,
    .bkstate = UISTATICFLEX_OPENING,
};

static void text_gameloop_proc()
{
    // check if created
    if (!pUIWND(&g_gamefrm.m_roledlg)->m_bcreateok) return;
    
    // roll state
    last_dlgstate = cur_dlgstate;
    
    // dump current state
    cur_dlgstate = (struct dlg_state_t) {
        .visible = pUIWND(&g_gamefrm.m_roledlg)->m_bvisible,
        .bkstate = g_gamefrm.m_roledlg.m_bk.m_state,
    };
    
    // check status
    if (!last_dlgstate.visible && cur_dlgstate.visible) {
        try_prepare(g_gamefrm.m_roledlg.m_static.m_buf._Ptr, g_gamefrm.m_roledlg.m_mode);
    } else if (last_dlgstate.bkstate != UISTATICFLEX_NORMAL && cur_dlgstate.bkstate == UISTATICFLEX_NORMAL) {
        try_start();
    } else if (last_dlgstate.visible && !cur_dlgstate.visible) {
        try_end();
    } else {
        try_idle(cur_dlgstate.visible && cur_dlgstate.bkstate == UISTATICFLEX_NORMAL);
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




// init and cleanup
static int plugin_init_flag = 0;

static void voice_prepal3destroy_hook()
{
    VoiceCleanup();
    plugin_init_flag = 0;
}

static void voice_postgamecreate_hook()
{
    // allocate static memory
    static struct VoiceToolkit toolkit;
    
    // set toolkit
    HMODULE mss32 = GetModuleHandle_check("MSS32.DLL");
    toolkit = (struct VoiceToolkit) {
        .patch_version = patch_version,
        .build_date = build_date,
        
        .hgfxwnd = PAL3_s_drvinfo.hgfxwnd,
        .phbink = &g_bink.m_hBink,
        .pcurtime = &g_CurrentTime,
        
        .ReportFatalError = ReportFatalError,
        
        .CalcStringCRC32 = CalcStringCRC32,
        
        .GetMusicMasterVolume = GetMusicMasterVolume,
        .SetMusicMasterVolume = SetMusicMasterVolume,
        .Get2DMasterVolume = Get2DMasterVolume,
        .Set2DMasterVolume = Set2DMasterVolume,
        .Get3DMasterVolume = Get3DMasterVolume,
        .Set3DMasterVolume = Set3DMasterVolume,
        
        .dig = SoundMgr_GetAudioMgr(SoundMgr_Inst())->h2DDriver,
        .AIL_open_stream = TOPTR(GetProcAddress(mss32, "_AIL_open_stream@12")),
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

    // dynlink voice plugin    
    HMODULE hPlugin = LoadLibrary_check(get_string_from_configfile("voiceplugin"));

    VoiceDLLAttached = TOPTR(GetProcAddress_check(hPlugin, "_VoiceDLLAttached@0"));
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
    
    // call dll attach callback
    VoiceDLLAttached();
}
