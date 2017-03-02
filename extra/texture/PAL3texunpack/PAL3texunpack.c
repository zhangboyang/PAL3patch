#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

// requirements:
//   put 'GBENGINE.DLL', 'MSS32.DLL', 'TOPO.DLL', 'IJL15.DLL' with this program
//   dir 'cpk' exists in current directory


static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    putchar('\n');
    //system("pause");
    exit(1);
    va_end(ap);
}

HMODULE LoadLibrary_safe(LPCTSTR lpFileName)
{
    HMODULE ret = LoadLibrary(lpFileName);
    if (!ret) fail("can't load library '%s'.", lpFileName);
    return ret;
}

FARPROC GetProcAddress_safe(HMODULE hModule, LPCSTR lpProcName)
{
    FARPROC ret = GetProcAddress(hModule, lpProcName);
    if (!ret) fail("can't find proc address for '%s'.", lpProcName);
    return ret;
}

#define SHA1_BYTE 20
#define SHA1_STR_SIZE (SHA1_BYTE * 2 + 1)

int sha1_hash_buffer(void *databuf, int datalen, char *strbuf)
{
	int ret = 0;
	unsigned char hashbuf[SHA1_BYTE];
	DWORD hashlen = sizeof(hashbuf);
	HCRYPTPROV hProv;
	HCRYPTHASH hHash;

	// use CryptAcquireContextA to make this program KernelEx (for Win9X) compatible
	if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		hProv = 0;
		goto done;
	}
	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		hHash = 0;
		goto done;
	}

	if (!CryptHashData(hHash, databuf, datalen, 0)) {
		goto done;
	}

	if (CryptGetHashParam(hHash, HP_HASHVAL, hashbuf, &hashlen, 0)) {
		int i;
		for (i = 0; i < SHA1_BYTE; i++) {
			sprintf(strbuf + i * 2, "%02x", (unsigned) hashbuf[i]);
		}
	} else {
		goto done;
	}

	ret = 1;
done:
	if (hHash) CryptDestroyHash(hHash);
	if (hProv) CryptReleaseContext(hProv, 0);
	return ret;
}


// these structures are read from PAL3A.PDB
enum ECPKMode { 
    CPKM_Null,
    CPKM_Normal,
    CPKM_FileMapping,
    CPKM_Overlapped,
    CPKM_End,
};
struct CPKHeader {
    ULONG dwLable;
    ULONG dwVersion;
    ULONG dwTableStart;
    ULONG dwDataStart;
    ULONG dwMaxFileNum;
    ULONG dwFileNum;
    ULONG dwIsFormatted;
    ULONG dwSizeOfHeader;
    ULONG dwValidTableNum;
    ULONG dwMaxTableNum;
    ULONG dwFragmentNum;
    ULONG dwPackageSize;
    ULONG dwReserved[0x14];
};
struct CPKTable {
    ULONG dwCRC;
    ULONG dwFlag;
    ULONG dwFatherCRC;
    ULONG dwStartPos;
    ULONG dwPackedSize;
    ULONG dwOriginSize;
    ULONG dwExtraInfoSize;
};
struct CPKFile {
    BYTE bValid;
    ULONG dwCRC;
    ULONG dwFatherCRC;
    int nTableIndex;
    void *lpMapAddress;
    void *lpStartAddress;
    ULONG dwOffset;
    BYTE bCompressed;
    void *lpMem;
    ULONG dwFileSize;
    ULONG dwPointer;
    struct CPKTable *pTableItem;
};
struct gbVFile;
struct CPK {
    ULONG m_dwAllocGranularity;
    enum ECPKMode m_eMode;
    struct CPKHeader m_CPKHeader;
    struct CPKTable m_CPKTable[0x8000];
    struct gbVFile *m_pgbVFile[0x8];
    BYTE m_bLoaded;
    ULONG m_dwCPKHandle;
    ULONG m_dwCPKMappingHandle;
    char m_szCPKFileName[0x104];
    int m_nOpenedFileNum;
};


// helper macros
#define TOPTR(addr) ((void *)(addr))
#define MAKE_THISCALL_FUNCPTR(addr, ret_type, this_type, ...) ((ret_type __fastcall (*)(this_type, int, ##__VA_ARGS__)) TOPTR(addr))
#define THISCALL_WRAPPER(func, this, ...) func(this, 0, ##__VA_ARGS__)

#define CPK_ctor(this) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_ctor, void, struct CPK *), this)
#define CPK_Load(this, cpkname) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_Load, BOOL, struct CPK *, const char *), this, cpkname)
#define CPK_GetTableIndexFromCRC(this, crc) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_GetTableIndexFromCRC, int, struct CPK *, unsigned long), this, crc)
#define CPK_Open(this, path) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_Open, struct CPKFile *, struct CPK *, const char *), this, path)
#define CPK_Close(this, fp) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_Close, BOOL, struct CPK *, struct CPKFile *), this, fp)
#define CPK_Read(this, buf, size, fp) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(pCPK_Read, BOOL, struct CPK *, void *, unsigned long, struct CPKFile *), this, buf, size, fp)

// function pointers
void *pCPK_ctor;
void *pCPK_Load;
void *pCPK_GetTableIndexFromCRC;
void *pCPK_Open;
void *pCPK_Close;
void *pCPK_Read;

// dynamic linker
void dynlink()
{
    HMODULE h_gbengine = LoadLibrary_safe("GBENGINE.DLL");
    pCPK_ctor = GetProcAddress_safe(h_gbengine, "??0CPK@@QAE@XZ");
    pCPK_Load = GetProcAddress_safe(h_gbengine, "?Load@CPK@@QAE_NPBD@Z");
    pCPK_GetTableIndexFromCRC = GetProcAddress_safe(h_gbengine, "?GetTableIndexFromCRC@CPK@@AAEHK@Z");
    pCPK_Open = GetProcAddress_safe(h_gbengine, "?Open@CPK@@QAEPAVCPKFile@@PBD@Z");
    pCPK_Close = GetProcAddress_safe(h_gbengine, "?Close@CPK@@QAE_NPAVCPKFile@@@Z");
    pCPK_Read = GetProcAddress_safe(h_gbengine, "?Read@CPK@@QAE_NPAXKPAVCPKFile@@@Z");
}





// CPK unpacker
const char *cpkfile;
struct CPK cpk;
char *cpk_namelist[0x8000]; // only file name
char *cpk_pathlist[0x8000]; // full path

void init_cpk()
{
    printf("init cpk ...\n");
    if (sizeof(struct CPK) != 0xe01bc) fail("sizeof(struct CPK) is wrong, current is %08X.", sizeof(struct CPK));
    CPK_ctor(&cpk);
    CPK_Load(&cpk, cpkfile);
}

int is_valid(int tindex)
{
    return cpk.m_CPKTable[tindex].dwExtraInfoSize != 0 && ((cpk.m_CPKTable[tindex].dwFlag & 1) && !(cpk.m_CPKTable[tindex].dwFlag & 0x10));
}

void make_cpk_namelist()
{
    printf("load names ...\n");
    FILE *fp = fopen(cpkfile, "rb");
    if (!fp) fail("can't open file '%s'.", cpkfile);
    memset(cpk_namelist, 0, sizeof(cpk_namelist));
    int i;
    for (i = 0; i < cpk.m_CPKHeader.dwValidTableNum; i++) {
        long name_offset = cpk.m_CPKTable[i].dwStartPos + cpk.m_CPKTable[i].dwPackedSize;
        long name_length = cpk.m_CPKTable[i].dwExtraInfoSize;
        if (!is_valid(i)) {
            printf("ignoring empty file entry %d.\n", i);
            continue;
        }
        
        if (fseek(fp, name_offset, SEEK_SET) != 0) {
            fail("can't seek to %08X.", name_offset);
        }
        cpk_namelist[i] = malloc(name_length);
        if (fread(cpk_namelist[i], 1, name_length, fp) != name_length) {
            fail("can't read next %08X bytes.", name_length);
        }
        
        //printf("%s\n", cpk_namelist[i]);
    }
    fclose(fp);
}

int get_full_path(int tindex, char *buf)
{
    int len;
    ULONG father_crc = cpk.m_CPKTable[tindex].dwFatherCRC;
    if (father_crc == 0) {
        if (buf) *buf = 0;
        len = 0;
    } else {
        len = get_full_path(CPK_GetTableIndexFromCRC(&cpk, father_crc), buf);
    }
    if (buf) {
        buf[len] = '\\';
        strcpy(buf + len + 1, cpk_namelist[tindex]);
    }
    len += 1 + strlen(cpk_namelist[tindex]);
    return len;
}
void make_cpk_pathlist()
{
    printf("make paths ...\n");
    memset(cpk_pathlist, 0, sizeof(cpk_pathlist));
    int i;
    for (i = 0; i < cpk.m_CPKHeader.dwValidTableNum; i++) {
        if (!is_valid(i)) continue;
        int len = get_full_path(i, NULL);
        cpk_pathlist[i] = malloc(len + 1);
        get_full_path(i, cpk_pathlist[i]);
    }
}

int is_dir(int tindex)
{
    // reference CPK::IsDir
    return (cpk.m_CPKTable[tindex].dwFlag >> 1) & 1;
}

int str_iendwith(const char *a, const char *b)
{
    int lena = strlen(a), lenb = strlen(b);
    return lena >= lenb && stricmp(a + lena - lenb, b) == 0;
}

void extract_files()
{
    int extract_cnt = 0;
    printf("extracting files ...\n");
    int i;
    FILE *logfp = fopen("cpk\\log.txt", "w");
    if (!logfp) fail("can't open log file for writing.");
    fprintf(logfp, "CPK \"%s\"\n", strrchr(cpkfile, '\\') ? strrchr(cpkfile, '\\') + 1 : cpkfile);
    fprintf(logfp, "BEGIN MAPPING\n");
    for (i = 0; i < cpk.m_CPKHeader.dwValidTableNum; i++) {
        if (i % 1000 == 0) printf("  progress %d/%d ...\n", i, (int) cpk.m_CPKHeader.dwValidTableNum);
        if (!is_valid(i)) continue;
        if (is_dir(i)) continue;
        char *path = cpk_pathlist[i] + 1;
        
        if (!str_iendwith(path, ".dds") && 
            !str_iendwith(path, ".tga") && 
            !str_iendwith(path, ".bmp") && 
            !str_iendwith(path, ".pcx") && 
            !str_iendwith(path, ".bit") && 
            !str_iendwith(path, ".jpg")) continue;

        int size = cpk.m_CPKTable[i].dwOriginSize;
        char *buf = malloc(size);
        //printf("path=%s\n", path);
        struct CPKFile *cpkfp = CPK_Open(&cpk, path);
        CPK_Read(&cpk, buf, size, cpkfp);
        CPK_Close(&cpk, cpkfp);
        char sha1buf[SHA1_STR_SIZE];
        if (!sha1_hash_buffer(buf, size, sha1buf)) {
            fail("sha1 failed on file %s.", path);
        }
        
        char cur_fn[1000];
        sprintf(cur_fn, "%s%s", sha1buf, strrchr(path, '.'));
        fprintf(logfp, "  TEXTURE \"%s\" SHA1 %s MAPTO \"%s\"\n", path, sha1buf, cur_fn);
        
        char cur_path[1000];
        sprintf(cur_path, "cpk\\%s", cur_fn);
        FILE *fp = fopen(cur_path, "wb");
        if (!fp) fail("can't open '%s'.", cur_path);
        if (fwrite(buf, 1, size, fp) != size) fail("write to '%s' failed.", cur_path);
        fclose(fp);
        
        extract_cnt++;
        
        free(buf);
    }
    fprintf(logfp, "END MAPPING\n");
    fclose(logfp);
    
    printf("extracted %d files.\n", extract_cnt);
}


int main(int argc, char *argv[])
{
    dynlink();

    if (argc != 2) {
        fail("usage: PAL3texunpack CPKNAME");
    }
    
    cpkfile = argv[1];
    printf("CPK = %s\n", cpkfile);
    init_cpk();
    make_cpk_namelist();
    make_cpk_pathlist();
    extract_files();
    
    printf("finished!\n");
    //system("pause");
    return 0;
}


