#ifndef PAL3PATCHCONFIG_PATCHVERSIONINFO
#define PAL3PATCHCONFIG_PATCHVERSIONINFO

#ifdef BUILD_FOR_PAL3
#define PATCH_VERSION_CSTR "v5.1"
#define PATCH_APPNAME _T("PAL3-PatchConfig")
#define PATCH_GAME_MUTEX "PAL3patch_GameMutex"
#define PATCH_TEMP_MUTEX "PAL3patch_TempMutex"
#define PATCH_TEMP_IN "PAL3patch.in"
#define PATCH_TEMP_OUT "PAL3patch.out"
#define GAME_EXE _T("PAL3.exe")
#endif


#ifdef BUILD_FOR_PAL3A
#define PATCH_VERSION_CSTR "v5.1"
#define PATCH_APPNAME _T("PAL3A-PatchConfig")
#define PATCH_GAME_MUTEX "PAL3Apatch_GameMutex"
#define PATCH_TEMP_MUTEX "PAL3Apatch_TempMutex"
#define PATCH_TEMP_IN "PAL3Apatch.in"
#define PATCH_TEMP_OUT "PAL3Apatch.out"
#define GAME_EXE _T("PAL3A.exe")
#endif


#define PATCH_VERSION _T(PATCH_VERSION_CSTR)


#define PATCH_WEBSITEBASE _T("https://pal3patch.com")
#define PATCH_UPDATEUA PATCH_APPNAME _T("/") PATCH_VERSION

#ifdef _DEBUG
#define PATCH_UPDATEURL _T("http://127.0.0.1/check-updates.txt")
#define PATCH_WEBSITEURL PATCH_UPDATEURL
#else
#ifdef BUILD_FOR_PAL3
#define PATCH_UPDATEURL PATCH_WEBSITEBASE _T("/pal3/check-updates.php")
#endif
#ifdef BUILD_FOR_PAL3A
#define PATCH_UPDATEURL PATCH_WEBSITEBASE _T("/pal3a/check-updates.php")
#endif
#define PATCH_WEBSITEURL PATCH_WEBSITEBASE _T("/")
#endif

extern const char pBuildDate[];
extern const char pVersionStr[];
extern const char pCompiler[];
extern const char *pFileHash[];


#endif
