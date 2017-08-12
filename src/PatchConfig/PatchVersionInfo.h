#ifndef PAL3PATCHCONFIG_PATCHVERSIONINFO
#define PAL3PATCHCONFIG_PATCHVERSIONINFO

#define PATCH_VERSION "v1.5"
#define PATCH_VERSION_TSTR _T(PATCH_VERSION)

#define PATCH_WEBSITE _T("https://pal3.zbyzbyzby.com")
#define PATCH_UPDATEUA _T("PAL3-PatchConfig/") PATCH_VERSION_TSTR
#ifdef _DEBUG
#define PATCH_UPDATEURL _T("http://127.0.0.1/check-updates.txt")
#else
#define PATCH_UPDATEURL _T("https://pal3.zbyzbyzby.com/pal3/check-updates.php")
#endif

extern const char pBuildDate[];
extern const char pVersionStr[];
extern const char pCompiler[];
extern const char *pFileHash[];


#endif