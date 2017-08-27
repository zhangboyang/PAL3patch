#ifndef PAL3PATCH_LOGGER_H
#define PAL3PATCH_LOGGER_H
#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define ERROR_FILE "PAL3patch.error.txt"
#define LOG_FILE "PAL3patch.log.txt"
#define MAXLOGLINES 10000
#define MAXWARNMSGBOXES 3

#define fail_with_extra_msg(extra_msg, fmt, ...) __fail(__FILE__, __LINE__, __func__, extra_msg, fmt, ## __VA_ARGS__)
#define fail(fmt, ...) fail_with_extra_msg(NULL, NULL, fmt, ## __VA_ARGS__)
extern void NORETURN __fail(const char *file, int line, const char *func, const wchar_t *extra_msg, const wchar_t *extra_msg_title, const char *fmt, ...);

#define warning(fmt, ...) __plog(1, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define plog(fmt, ...) __plog(0, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
extern void __plog(int is_warning, const char *file, int line, const char *func, const char *fmt, ...);

#endif
#endif
