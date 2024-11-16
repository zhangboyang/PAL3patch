#ifndef PAL3APATCH_MISC_H
#define PAL3APATCH_MISC_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int is_win9x(void);
extern PATCHAPI int is_laa(void);
extern PATCHAPI void NORETURN die(int status);

extern PATCHAPI int is_spacechar(char ch);
extern PATCHAPI int is_digitchar(char ch);
extern PATCHAPI int str2int(const char *valstr);
extern PATCHAPI double str2double(const char *valstr);
extern PATCHAPI int str_startswith(const char *a, const char *b);
extern PATCHAPI int str_istartswith(const char *a, const char *b);
extern PATCHAPI int str_endswith(const char *a, const char *b);
extern PATCHAPI int str_iendswith(const char *a, const char *b);
extern PATCHAPI int str_remove_utf8_bom(char *str);
extern PATCHAPI char *str_rtrim(char *str, const char *charlist);
extern PATCHAPI char *str_ltrim(char *str, const char *charlist);
extern PATCHAPI char *str_trim(char *str, const char *charlist);
extern PATCHAPI char *strtok_r(char *str, const char *delim, char **saveptr);
extern PATCHAPI char *str_tolower(char *s);
extern PATCHAPI wchar_t *wcs_tolower(wchar_t *s);
extern PATCHAPI int stricmp_C(const char *a, const char *b);
extern PATCHAPI int wcsicmp_C(const wchar_t *a, const wchar_t *b);
extern PATCHAPI int strnicmp_C(const char *a, const char *b, size_t n);
extern PATCHAPI int wcsnicmp_C(const wchar_t *a, const wchar_t *b, size_t n);
extern PATCHAPI int iabs(int x);
extern PATCHAPI int imax(int a, int b);
extern PATCHAPI int imin(int a, int b);
extern PATCHAPI double fbound(double x, double low, double high);
extern PATCHAPI int fcmp(double a, double b);
extern PATCHAPI int MessageBoxW_format(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, ...); 
extern PATCHAPI int MessageBoxW_utf8format(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, ...);
extern PATCHAPI HMODULE GetModuleHandle_check(LPCSTR lpModuleName);
extern PATCHAPI HMODULE LoadLibrary_check(LPCSTR lpFileName);
extern PATCHAPI FARPROC GetProcAddress_check(HMODULE hModule, LPCSTR lpProcName);



#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#endif
#endif
