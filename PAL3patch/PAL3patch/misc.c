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
    snprintf(buf, sizeof(buf), "FILE: %s\nLINE: %d\nFUNC: %s\n\n", file, line, func);
    len = strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    MessageBoxA(NULL, buf, NULL, MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 1);
    va_end(ap);
}
