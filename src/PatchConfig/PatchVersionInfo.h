#ifndef PAL3PATCHCONFIG_PATCHVERSIONINFO
#define PAL3PATCHCONFIG_PATCHVERSIONINFO

#ifdef BUILD_FOR_PAL3
#define PATCH_VERSION_CSTR "v3.1"
#define PATCH_APPNAME _T("PAL3-PatchConfig")
#define PATCH_MUTEXNAME "PAL3patch_GameMutex"
#endif


#ifdef BUILD_FOR_PAL3A
#define PATCH_VERSION_CSTR "v3.1"
#define PATCH_APPNAME _T("PAL3A-PatchConfig")
#define PATCH_MUTEXNAME "PAL3Apatch_GameMutex"
#endif


#define PATCH_VERSION _T(PATCH_VERSION_CSTR)


#define PATCH_WEBSITEBASE _T("https://pal3.zbyzbyzby.com")
#define PATCH_UPDATEUA PATCH_APPNAME _T("/") PATCH_VERSION

#ifdef _DEBUG
#define PATCH_UPDATEURL _T("http://127.0.0.1/check-updates.txt")
#define PATCH_WEBSITEURL _T("http://127.0.0.1/check-updates.txt")
#else
#ifdef BUILD_FOR_PAL3
#define PATCH_UPDATEURL _T("https://pal3.zbyzbyzby.com/pal3/check-updates.php")
#endif
#ifdef BUILD_FOR_PAL3A
#define PATCH_UPDATEURL _T("https://pal3.zbyzbyzby.com/pal3a/check-updates.php")
#endif
#define PATCH_WEBSITEURL PATCH_WEBSITEBASE _T("/")
#endif

extern const char pBuildDate[];
extern const char pVersionStr[];
extern const char pCompiler[];
extern const char *pFileHash[];


#endif
