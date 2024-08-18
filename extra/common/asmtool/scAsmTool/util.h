#pragma once

extern const char built_on[];


extern FILE *safe_fopen(const char *path, const char *mode);

extern size_t _safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream, const char *msg);
#define safe_fread(ptr, size, nmemb, stream) _safe_fread(ptr, size, nmemb, stream, "fread(" #ptr ", " #size ", " #nmemb ", " #stream ")")

extern int _safe_fseek(FILE *stream, long offset, int whence, const char *msg);
#define safe_fseek(stream, offset, whence) _safe_fseek(stream, offset, whence, "fseek(" #stream ", " #offset ", " #whence ")")

extern const char *getpathfilepart(const char *path);
