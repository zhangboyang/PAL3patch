#ifndef PAL3PATCH_VOICEPLUGIN_H
#define PAL3PATCH_VOICEPLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SAMPLE *HSAMPLE;
typedef struct _STREAM *HSTREAM;
typedef struct _DIG_DRIVER *HDIGDRIVER;
typedef struct h3DPOBJECT *H3DPOBJECT;
#define AILCALLBACK WINAPI
typedef void (AILCALLBACK FAR* AILSTREAMCB)(HSTREAM stream);

typedef struct BINK {
	unsigned int Width;
	unsigned int Height;
	unsigned int Frames;
	unsigned int FrameNum;
	unsigned int LastFrameNum;
	unsigned int FrameRate;
	unsigned int FrameRateDiv;
	unsigned int ReadError;
	unsigned int OpenFlags;
	unsigned int BinkType;
	unsigned int Size;
	unsigned int FrameSize;
	unsigned int SndSize;
} BINK, *HBINK;

#define BINKSURFACE32 3
#define BINKCOPYALL 0x80000000L







// see patch_voice.c for details

struct MiscToolkit {
    // error handling functions
    void (WINAPI *ReportFatalError)(const char *msg);
    
    // hash functions
    unsigned int (WINAPI *CalcStringCRC32)(const char *str);
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
    struct VolumeToolkit *vol;
    struct BinkToolkit *bik;
    struct MSSToolkit *mss;
};









enum {
    ROLEDLG_CLOSED,  // End() -> Prepare()
    ROLEDLG_OPENING, // Prepare() -> Start()
    ROLEDLG_OPENED,  // Start() -> End()
};

#define DLLEXPORT __declspec(dllexport)

// plugin management functions
DLLEXPORT void WINAPI VoiceDLLAttached(void);
DLLEXPORT void WINAPI VoiceInit(struct VoiceToolkit *toolkit);
DLLEXPORT void WINAPI VoiceCleanup(void);
DLLEXPORT void WINAPI GamePause(void);
DLLEXPORT void WINAPI GameResume(void);

// dialog text functions
DLLEXPORT void WINAPI TextIdle(int state);
DLLEXPORT void WINAPI TextPrepare(const char *text, int mode);
DLLEXPORT void WINAPI TextStart(void);
DLLEXPORT void WINAPI TextStop(void);

// caption functions
DLLEXPORT void WINAPI CaptionIdle(int state);
DLLEXPORT void WINAPI CaptionPrepare(const char *tex);
DLLEXPORT void WINAPI CaptionStart(void);
DLLEXPORT void WINAPI CaptionStop(void);

// movie functions
DLLEXPORT void WINAPI MovieIdle(int state);
DLLEXPORT void WINAPI MoviePrepare(const char *movie);
DLLEXPORT void WINAPI MovieStart(void);
DLLEXPORT void WINAPI MovieStop(void);

#ifdef __cplusplus
}
#endif

#endif
