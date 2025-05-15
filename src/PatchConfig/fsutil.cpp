#include "stdafx.h"

int create_dir(const char *dirpath)
{
    return !!CreateDirectoryA(dirpath, NULL);
}

int dir_exists(const char *dirpath)
{
    DWORD dwAttrs = GetFileAttributesA(dirpath);
    return dwAttrs != INVALID_FILE_ATTRIBUTES && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}

int file_exists(const char *filepath)
{
    return GetFileAttributesA(filepath) != INVALID_FILE_ATTRIBUTES;
}

int reset_attrib(const char *filepath)
{
    return !!SetFileAttributesA(filepath, FILE_ATTRIBUTE_ARCHIVE);
}

#define ROBUST_MAXTRY 10
#define ROBUST_WAIT   100

FILE *robust_fopen(const char *filename, const char *mode)
{
    FILE *ret;
    int i;
    for (i = 0; i < ROBUST_MAXTRY; i++) {
        if (i) Sleep(ROBUST_WAIT);
        reset_attrib(filename);
        ret = fopen(filename, mode);
        if (ret || errno == ENOENT) break;
    }
    return ret;
}

int safe_fclose(FILE **fp)
{
    int ret = 0;
    if (*fp) {
        ret = fclose(*fp);
        *fp = NULL;
    }
    return ret;
}

int batch_delete(const char *filepath[], int n)
{
    int i, j;
    for (i = 0; i < ROBUST_MAXTRY; i++) {
        if (i) Sleep(ROBUST_WAIT);
        int success = 1;
        for (j = 0; j < n; j++) {
            reset_attrib(filepath[j]);
            success = (_unlink(filepath[j]) == 0 || errno == ENOENT) && success;
        }
        if (success) return 1;
    }
    return 0;
}
