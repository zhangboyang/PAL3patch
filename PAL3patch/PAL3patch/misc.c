// misc functions
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "common.h"

void __fail(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[MAXLINE];
    int len;
    snprintf(buf, sizeof(buf), "  file: %s\n  line: %d\n  func: %s\nmessage:\n  ", file, line, func);
    len = strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    OutputDebugString(buf);
    FILE *fp = fopen(ERROR_FILE, "w");
    if (fp) {
        fputs("build information:\n", fp);
        fputs(build_info, fp);
        fputs("error details:\n", fp);
        fputs(buf, fp);
        fputc('\n', fp);
        fclose(fp);
    }
    MessageBoxA(NULL, buf + len, NULL, MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 1);
    va_end(ap);
}
