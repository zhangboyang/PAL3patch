#include "common.h"

// file system functions

int utf8_filepath_to_wstr_fullpath(const char *filepath, wchar_t *buf, size_t bufsize, wchar_t **filepart)
{
    int ret = 0;
    wchar_t *wstr_managed = NULL;
    wchar_t *filepart_out;
    DWORD r;
    
    cs2wcs_managed(filepath, CP_UTF8, &wstr_managed);
    r = GetFullPathNameW(wstr_managed, bufsize, buf, &filepart_out);
    if (r == 0 || r >= bufsize) goto done;
    
    if (filepart) *filepart = filepart_out;
    ret = 1;
    
done:
    free(wstr_managed);
    return ret;
}

int replace_filepart_with_relpath(const wchar_t *filepath, const wchar_t *relpath, wchar_t *buf, size_t bufsize, wchar_t **filepart)
{
    int ret = 0;
    wchar_t wbuf[MAXLINE];
    wchar_t *filepart_out;
    DWORD r;
    
    // convert to fullpath and remove file part
    r = GetFullPathNameW(filepath, MAXLINE, wbuf, &filepart_out);
    if (r == 0 || r >= MAXLINE) goto done;
    if (filepart_out) *filepart_out = 0;

    // append relpath
    if (wcslen(wbuf) + wcslen(relpath) >= MAXLINE) goto done;
    wcscat(wbuf, relpath);
    
    // get fullpath
    r = GetFullPathNameW(wbuf, bufsize, buf, &filepart_out);
    if (r == 0 || r >= bufsize) goto done;
    if (filepart) *filepart = filepart_out;
    ret = 1;
    
done:
    return ret;
}

char *get_filepart(const char *filepath)
{
    char *t;
    char *filepart = (char *) filepath;
    
    t = strrchr(filepart, '\\');
    if (t) filepart = t + 1;
    t = strrchr(filepart, '/');
    if (t) filepart = t + 1;
    
    return filepart;
}

wchar_t *get_wfilepart(const wchar_t *wfilepath)
{
    wchar_t *t;
    wchar_t *wfilepart = (wchar_t *) wfilepath;
    
    t = wcsrchr(wfilepart, L'\\');
    if (t) wfilepart = t + 1;
    t = wcsrchr(wfilepart, L'/');
    if (t) wfilepart = t + 1;
    
    return wfilepart;
}

int is_relpath(const char *filepath)
{
    return !(*filepath == '\\' || (filepath[0] && filepath[1] == ':'));
}

// read file as a string, will skip utf8 bom, return NULL if failed
// will allocate memory, don't forget to free
char *read_file_as_cstring(const char *filepath)
{
    char *filedata = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwSize, dwRead;
    wchar_t fullpath[MAXLINE];
    
    // convert filename
    if (!utf8_filepath_to_wstr_fullpath(filepath, fullpath, MAXLINE, NULL)) goto fail;
    
    // open file
    hFile = CreateFileW(fullpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) goto fail;
    
    // get file size
    dwSize = GetFileSize(hFile, NULL);
    if (dwSize == INVALID_FILE_SIZE) goto fail;
    
    // allocate memory and read file
    filedata = malloc(dwSize + 1);
    if (!ReadFile(hFile, filedata, dwSize, &dwRead, NULL) || dwSize != dwRead) goto fail;
    filedata[dwSize] = 0;
    
done:
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return filedata;
fail:
    free(filedata); filedata = NULL;
    goto done;
}


static int wfilename_cmp(const void *a, const void *b)
{
    const wchar_t * const *pa = a;
    const wchar_t * const *pb = b;
    return wcsicmp(*pa, *pb);
}

// enum files in given directory
//  return value is total file count
//   if pattern == NULL, dirpath is used as search pattern
//   if func == NULL, no callback function will be invoked
int enum_files(const char *dirpath, const char *pattern, void (*func)(const char *filepath, void *arg), void *arg)
{
    int sum = 0;
    char *cstr_managed = NULL;
    char buf[MAXLINE];
    wchar_t wbuf[MAXLINE];
    wchar_t searchpatt[MAXLINE];
    wchar_t *searchpatt_filepart;
    wchar_t *filelist[MAXLINE];
    int nr_filelist = 0;
    int i;
    WIN32_FIND_DATAW FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    
    // construct utf8 search pattern
    if (pattern) {
        snprintf(buf, sizeof(buf), "%s\\%s", dirpath, pattern);
        if (strlen(buf) >= MAXLINE - 1) goto fail;
    } else {
        if (strlen(dirpath) >= MAXLINE) goto fail;
        strcpy(buf, dirpath);
    }
    
    
    // convert to unicode
    if (!utf8_filepath_to_wstr_fullpath(buf, searchpatt, MAXLINE, &searchpatt_filepart)) goto fail;
    if (!searchpatt_filepart) goto fail;
    
    // do enum
    hFind = FindFirstFileW(searchpatt, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        // no such file found
        sum = 0;
        goto done;
    }
    do {
        // skip dirs
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        
        // construct filename
        if (!replace_filepart_with_relpath(searchpatt, FindFileData.cFileName, wbuf, MAXLINE, NULL)) goto fail;
        
        // add to file list
        if (nr_filelist >= MAXLINE) goto fail;
        filelist[nr_filelist++] = wcsdup(wbuf);
        
    } while (FindNextFileW(hFind, &FindFileData));
    
    // sort files
    qsort(filelist, nr_filelist, sizeof(wchar_t *), wfilename_cmp);
    
    for (i = 0; i < nr_filelist; i++) {
        // convert to utf8 and invoke function
        wcs2cs_managed(filelist[i], CP_UTF8, &cstr_managed);
        if (func) func(cstr_managed, arg);
    }
    sum = nr_filelist;
    
done:
    for (i = 0; i < nr_filelist; i++) {
        free(filelist[i]);
    }
    free(cstr_managed);
    if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);
    return sum;
fail:
    sum = -1;
    goto done;
}
