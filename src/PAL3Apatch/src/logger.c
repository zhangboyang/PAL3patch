#include "common.h"

// log framework

static wchar_t *msgbox_buf = NULL;
#define free_msgbox_buf() do { free(msgbox_buf); msgbox_buf = NULL; } while (0)

static void write_logfile_header(FILE *fp)
{
    fputs(UTF8_BOM_STR, fp);
    
    fputs("build information:\n", fp);
    fputs(build_info, fp);
    
    fputs("patch configuration:\n", fp);
    dump_all_config(fp);
}

void NORETURN __fail(const char *file, int line, const char *func, const wchar_t *extra_msg, const wchar_t *extra_msg_title, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    char msgbuf[MAXLINE];
    int len;
    
    snprintf(msgbuf, sizeof(msgbuf), "  file: %s\n  line: %d\n  func: %s\nmessage:\n  ", file, line, func);
    len = strlen(msgbuf);
    vsnprintf(msgbuf + len, sizeof(msgbuf) - len, fmt, ap);
    OutputDebugString(msgbuf); OutputDebugString("\n");
    FILE *fp = fopen(ERROR_FILE, "w");
    if (fp) {
        write_logfile_header(fp);
        
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);
        snprintf(buf, sizeof(buf), "  %04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu\n", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
        fputs("timestamp:\n", fp);
        fputs(buf, fp);
        
        fputs("error details:\n", fp);
        fputs(msgbuf, fp);
        fputc('\n', fp);
        fclose(fp);
    }
    try_goto_desktop();
    
    MessageBoxW(NULL, cs2wcs_managed(msgbuf + len, CP_UTF8, &msgbox_buf), L"PAL3Apatch", MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
    free_msgbox_buf();
    if (extra_msg) {
        MessageBoxW(NULL, extra_msg, extra_msg_title, MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
    }
    
    die(1);
    va_end(ap);
}

static long long plog_lines = 0;
static long long plog_msgboxes = 0;
void __plog(int is_warning, const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    char msgbuf[MAXLINE];
    int len;
    snprintf(msgbuf, sizeof(msgbuf), "  file: %s\n  line: %d\n  func: %s\nmessage:\n  ", file, line, func);
    len = strlen(msgbuf);
    vsnprintf(msgbuf + len, sizeof(msgbuf) - len, fmt, ap);
    OutputDebugString(msgbuf); OutputDebugString("\n");
    plog_lines++;
    if (plog_lines <= MAXLOGLINES) {
        FILE *fp = fopen(LOG_FILE, plog_lines > 1 ? "a" : "w");
        if (fp) {
            if (plog_lines == 1) {
                write_logfile_header(fp);
                
                fputs("========== start ==========\n\n", fp);
            }
            
            SYSTEMTIME SystemTime;
            GetLocalTime(&SystemTime);
            snprintf(buf, sizeof(buf), "  %04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu\n", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
            fputs("timestamp:\n", fp);
            fputs(buf, fp);
            
            fputs("details:\n", fp);
            fputs(msgbuf, fp);
            fputs("\n\n\n", fp);
            if (plog_lines >= MAXLOGLINES) {
                fputs("too many messages, log truncated.\n", fp);
            }
            fclose(fp);
        }
        if (is_warning) {
            if (plog_msgboxes + 1 <= MAXWARNMSGBOXES) {
                plog_msgboxes++;
                if (plog_msgboxes >= MAXWARNMSGBOXES) {
                    strncat(msgbuf, "\n\nmax messagebox limit reached, suppressing further messageboxes.", sizeof(msgbuf) - strlen(msgbuf) - 1);
                    msgbuf[sizeof(msgbuf) - 1] = '\0';
                }
                try_goto_desktop();
                MessageBoxW(NULL, cs2wcs_managed(msgbuf + len, CP_UTF8, &msgbox_buf), L"PAL3Apatch", MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
                free_msgbox_buf();
            }
        }
    }
    
    va_end(ap);
}
