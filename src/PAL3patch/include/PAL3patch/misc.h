#ifndef PAL3PATCH_MISC_H
#define PAL3PATCH_MISC_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int is_win9x(void);
extern PATCHAPI void NORETURN die(int status);

extern PATCHAPI int str2int(const char *valstr);
extern PATCHAPI double str2double(const char *valstr);
extern PATCHAPI int str_iendwith(const char *a, const char *b);
extern PATCHAPI int iabs(int x);
extern PATCHAPI int imax(int a, int b);
extern PATCHAPI int imin(int a, int b);
extern PATCHAPI double fbound(double x, double low, double high);
extern PATCHAPI int fcmp(double a, double b);
extern PATCHAPI HMODULE GetModuleHandle_check(LPCSTR lpModuleName);
extern PATCHAPI HMODULE LoadLibrary_check(LPCSTR lpFileName);
extern PATCHAPI HMODULE LoadLibraryW_check(LPCWSTR lpFileName);
extern PATCHAPI FARPROC GetProcAddress_check(HMODULE hModule, LPCSTR lpProcName);



#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define ERROR_FILE "PAL3patch.error.txt"
#define WARNING_FILE "PAL3patch.log.txt"
#define MAXLOGLINES 10000
#define MAXWARNMSGBOXES 3

#define fail(fmt, ...) __fail(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void NORETURN __fail(const char *file, int line, const char *func, const char *fmt, ...);
#define warning(fmt, ...) __plog(1, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define plog(fmt, ...) __plog(0, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __plog(int is_warning, const char *file, int line, const char *func, const char *fmt, ...);

#endif
#endif
