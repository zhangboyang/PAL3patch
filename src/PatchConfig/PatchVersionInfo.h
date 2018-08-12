#ifndef PAL3PATCHCONFIG_PATCHVERSIONINFO
#define PAL3PATCHCONFIG_PATCHVERSIONINFO

#ifdef BUILD_FOR_PAL3
#define PATCH_VERSION_CSTR "v1.7"
#define PATCH_APPNAME _T("PAL3-PatchConfig")
#endif


#ifdef BUILD_FOR_PAL3A
#define PATCH_VERSION_CSTR "v1.1"
#define PATCH_APPNAME _T("PAL3A-PatchConfig")
#endif


#define PATCH_VERSION _T(PATCH_VERSION_CSTR)


#define PATCH_WEBSITE_BASE _T("https://pal3.zbyzbyzby.com")
#define PATCH_WEBSITE PATCH_WEBSITE_BASE _T("/?appname=") PATCH_APPNAME _T("&version=") PATCH_VERSION
#define PATCH_UPDATEUA PATCH_APPNAME _T("/") PATCH_VERSION

#ifdef _DEBUG
#define PATCH_UPDATEURL _T("http://127.0.0.1/check-updates.txt")
#else
#ifdef BUILD_FOR_PAL3
#define PATCH_UPDATEURL _T("https://pal3.zbyzbyzby.com/pal3/check-updates.php")
#endif
#ifdef BUILD_FOR_PAL3A
#define PATCH_UPDATEURL _T("https://pal3.zbyzbyzby.com/pal3a/check-updates.php")
#endif
#endif

extern const char pBuildDate[];
extern const char pVersionStr[];
extern const char pCompiler[];
extern const char *pFileHash[];


#endif