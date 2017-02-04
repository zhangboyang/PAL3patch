#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)

#define MAXLINE 4096
#define MAXLINEFMT "%4095s"

#define ROUND_UP_INC(value, graduate) ((value) - (value) % (graduate) + (graduate))
#define ROUND_UP(value, graduate) ((value) % (graduate) == 0 ? (value) : ROUND_UP_INC(value, graduate))

#define BUFSIZE (16 * 1048576)


unsigned char pe[BUFSIZE];
unsigned peptr;
DWORD *ppeimagesize, *ppeheaderssize;
PIMAGE_DATA_DIRECTORY ppedirectory;

// MANUALLY input PE data here
// the dos header, dos stub, rich signature of template file will be used
#define PE_TEMPLATE_FILE "PAL3.EXE"
#define PE_OUTPUT_FILE "PAL3unpacked.exe"
#define PE_TOTAL_SECTIONS 4
#define PE_IMAGEBASE 0x00400000
#define PE_CODEBASE 0x00001000
#define PE_SECTIONALIGN 0x1000
#define PE_FILEALIGN 0x1000
#define PE_ENTRY_RVA (0x55507C - PE_IMAGEBASE)
#define PE_IAT_RVA (0x0056A000 - PE_IMAGEBASE)
#define PE_IMPORT_RVA (0x00011C00 + 0x0056A000 - PE_IMAGEBASE)
#define PE_IMPORT_DLLCOUNT 7
#define PE_IMPORT_SIZE ((PE_IMPORT_DLLCOUNT + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR))

// section data
struct {
    char name[MAXLINE];
    unsigned vsize; // virtual size
    unsigned psize; // raw size
    unsigned base;
    unsigned flags;
    DWORD *prawdataptr;
    unsigned char data[BUFSIZE];
} sdata[PE_TOTAL_SECTIONS];
unsigned code_size; // SizeOfCode
unsigned data_size; // SizeOfInitializedData
unsigned iat_size, import_size;

void read_sdata()
{
    memset(sdata, 0, sizeof(sdata));
    code_size = 0;
    data_size = 0;
    
    FILE *fp;
    int sid;
    
    // load section name and base
    fp = fopen("dump.summary.txt", "r");
    assert(fp);
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        fscanf(fp, MAXLINEFMT, sdata[sid].name);
        fscanf(fp, "%x%*x", &sdata[sid].base); // 'size' is ignored!
        sdata[sid].base -= PE_IMAGEBASE;
        if (sid >= 1) assert(sdata[sid].base > sdata[sid - 1].base); // order is important
        
        if (strcmp(sdata[sid].name, ".text") == 0) {
            assert(sid == 0);
            sdata[sid].flags = 0x60000020; // Code, Execute, Read
        } else if (strcmp(sdata[sid].name, ".rdata") == 0) {
            assert(sid == 1);
            sdata[sid].flags = 0x40000040; // Initialized Data, Read Only
        } else if (strcmp(sdata[sid].name, ".data") == 0) {
            assert(sid == 2);
            sdata[sid].flags = 0xC0000040; // Initialized Data, Read Write
        } else if (strcmp(sdata[sid].name, ".rsrc") == 0) {
            assert(sid == 3);
            sdata[sid].flags = 0x40000040; // Initialized Data, Read Only
        } else assert(0);
    }
    fclose(fp);
    
    // load data for each section
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        char fn[MAXLINE];
        snprintf(fn, sizeof(fn), "dump%s.fixed", sdata[sid].name); // try fixed first
        FILE *fp = fopen(fn, "rb");
        if (fp == NULL) {
            snprintf(fn, sizeof(fn), "dump%s", sdata[sid].name);
            fp = fopen(fn, "rb");
        }
        assert(fp);
        int c;
        unsigned len = 0;
        while (len < sizeof(sdata[sid].data) && (c = fgetc(fp)) != EOF)
            sdata[sid].data[len++] = c;
        assert(len < sizeof(sdata[sid].data));
        sdata[sid].vsize = len;
        sdata[sid].psize = ROUND_UP(len, PE_SECTIONALIGN);
        fclose(fp);


        // calculate size for each section
        if (strcmp(sdata[sid].name, ".text") == 0) {
            code_size += sdata[sid].psize;
            assert(sdata[sid].base == PE_CODEBASE);
        } else {
            data_size += sdata[sid].psize;
            if (strcmp(sdata[sid].name, ".data") == 0) {
                sdata[sid].vsize = sdata[sid + 1].base - sdata[sid].base;
            } else if (strcmp(sdata[sid].name, ".rdata") == 0) {
                assert(PE_CODEBASE + code_size == sdata[sid].base);
            }
        }
        
        printf("%-6s  BASE %08X  VSIZE %08X  PSIZE %08X  FROM %s\n", sdata[sid].name, sdata[sid].base, sdata[sid].vsize, sdata[sid].psize, fn);
    }
    
    printf("SUMMARY:  CODESIZE %08X  DATASIZE %08X\n", code_size, data_size);
}

unsigned read_file(unsigned char *ptr, const char *fn)
{
    FILE *fp = fopen(fn, "rb");
    assert(fp);
    int c;
    unsigned cnt = 0;
    while ((c = fgetc(fp)) != EOF) { *ptr++ = c; cnt++; }
    fclose(fp);
    return cnt;
}

void read_import()
{
    int sid;
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        if (strcmp(sdata[sid].name, ".rdata") == 0) {
            iat_size = read_file(sdata[sid].data + (PE_IAT_RVA - sdata[sid].base), "iat.bin");
            import_size = read_file(sdata[sid].data + (PE_IMPORT_RVA - sdata[sid].base), "import.bin");
            printf("IMPORTFIX:  IATSIZE %08X  IMPORTSIZE %08X\n", iat_size, import_size);
            data_size -= sdata[sid].psize;
            sdata[sid].vsize = (PE_IMPORT_RVA - sdata[sid].base) + import_size;
            data_size += sdata[sid].psize;
            printf("            SECTION .rdata NEW VSIZE %08X\n", sdata[sid].vsize);
            printf("            NEW DATASIZE %08X\n", data_size);
            assert(ROUND_UP(sdata[sid].vsize, PE_SECTIONALIGN) == sdata[sid].psize);
        }
    }
}

unsigned make_dosheader()
{
    size_t sret;
    int iret;
    IMAGE_DOS_HEADER doshdr;
    FILE *fp = fopen(PE_TEMPLATE_FILE, "rb");
    assert(fp);
    sret = fread(&doshdr, sizeof(IMAGE_DOS_HEADER), 1, fp);
    assert(sret == 1);
    iret = fseek(fp, 0, SEEK_SET);
    assert(iret == 0);
    sret = fread(pe + peptr, 1, doshdr.e_lfanew, fp);
    assert(sret == doshdr.e_lfanew);
    fclose(fp);
    printf("READ DOSHEADER, LEN = %08X\n", sret);
    return sret;
}

unsigned make_ntheader()
{
    PIMAGE_NT_HEADERS pnthdr = (void *)(pe + peptr);
    PIMAGE_FILE_HEADER pfilehdr = &pnthdr->FileHeader;
    PIMAGE_OPTIONAL_HEADER popthdr = &pnthdr->OptionalHeader;
    
    assert(PE_FILEALIGN == PE_SECTIONALIGN); // there might be some mistake in size calculation
    
    pnthdr->Signature = 0x4550;
    
    pfilehdr->Machine = IMAGE_FILE_MACHINE_I386;
    pfilehdr->NumberOfSections = PE_TOTAL_SECTIONS;
    pfilehdr->TimeDateStamp = time(NULL);
    pfilehdr->SizeOfOptionalHeader = 0xE0;
    pfilehdr->Characteristics = IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LINE_NUMS_STRIPPED | IMAGE_FILE_LOCAL_SYMS_STRIPPED | IMAGE_FILE_32BIT_MACHINE;
    
    popthdr->Magic = IMAGE_NT_OPTIONAL_HDR_MAGIC;
    popthdr->MajorLinkerVersion = 6;
    popthdr->SizeOfCode = code_size;
    popthdr->SizeOfInitializedData = data_size;
    popthdr->AddressOfEntryPoint = PE_ENTRY_RVA;
    popthdr->BaseOfCode = PE_CODEBASE;
    popthdr->BaseOfData = PE_CODEBASE + code_size;
    popthdr->ImageBase = PE_IMAGEBASE;
    popthdr->SectionAlignment = PE_SECTIONALIGN;
    popthdr->FileAlignment = PE_FILEALIGN;
    popthdr->MajorOperatingSystemVersion = 4;
    popthdr->MajorSubsystemVersion = 4;
    ppeimagesize = &popthdr->SizeOfImage; // SizeOfImage will be filled later
    ppeheaderssize = &popthdr->SizeOfHeaders; // SizeOfHeaders will be filled later
    popthdr->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
    popthdr->SizeOfStackReserve = 0x100000;
    popthdr->SizeOfStackCommit = 0x1000;
    popthdr->SizeOfHeapReserve = 0x100000;
    popthdr->SizeOfHeapCommit = 0x1000;
    popthdr->NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    ppedirectory = popthdr->DataDirectory; // DataDirectory will be filled later

    int sid;
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        if (strcmp(sdata[sid].name, ".rsrc") == 0) {
            // fill the resource directory
            ppedirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = sdata[sid].base;
            ppedirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = sdata[sid].vsize;
        } else if (strcmp(sdata[sid].name, ".rdata") == 0) {
            // MANUALLY enter import and IAT data here
            // fill the import directory
            ppedirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = PE_IMPORT_RVA;
            ppedirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = PE_IMPORT_SIZE;
            // fill the IAT directory
            ppedirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = PE_IAT_RVA;
            ppedirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = iat_size;
        }
    }
    return sizeof(IMAGE_NT_HEADERS);
}

unsigned make_sectionheader()
{
    int sid;
    unsigned ret = 0;
    PIMAGE_SECTION_HEADER pshdr = (void *)(pe + peptr);
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        strncpy(pshdr[sid].Name, sdata[sid].name, IMAGE_SIZEOF_SHORT_NAME);
        pshdr[sid].Misc.VirtualSize = sdata[sid].vsize;
        pshdr[sid].VirtualAddress = sdata[sid].base;
        pshdr[sid].SizeOfRawData = sdata[sid].psize;
        sdata[sid].prawdataptr = &pshdr[sid].PointerToRawData; // PointerToRawData will be filled later
        pshdr[sid].Characteristics = sdata[sid].flags;
        ret += sizeof(IMAGE_SECTION_HEADER);
    }
    return ret;
}

unsigned make_sections()
{
    int sid;
    unsigned ptr = peptr;
    for (sid = 0; sid < PE_TOTAL_SECTIONS; sid++) {
        *sdata[sid].prawdataptr = ptr;
        memcpy(pe + ptr, sdata[sid].data, sdata[sid].psize);
        printf("%-6s  OFFSET %08X  SIZE %08X\n", sdata[sid].name, ptr, ROUND_UP(sdata[sid].psize, PE_FILEALIGN));

        ptr = ROUND_UP(ptr + sdata[sid].psize, PE_FILEALIGN);
    }
    return ptr - peptr;
}

int main()
{
    printf("====== LOAD SECTIONS ======\n");
    read_sdata();
    read_import();
    
    printf("====== MAKE PE ======\n");
    memset(pe, 0, sizeof(pe));
    peptr = 0;
    peptr += make_dosheader();
    peptr += make_ntheader();
    peptr += make_sectionheader();
    peptr = ROUND_UP(peptr, PE_FILEALIGN);
    *ppeheaderssize = peptr;
    printf("HEADER SIZE %08X\n", peptr);
    peptr = ROUND_UP(peptr, PE_FILEALIGN);
    peptr += make_sections();
    peptr = ROUND_UP(peptr, PE_FILEALIGN);
    *ppeimagesize = sdata[PE_TOTAL_SECTIONS - 1].base + sdata[PE_TOTAL_SECTIONS - 1].psize;
    printf("IMAGE SIZE %08X\n", (unsigned) *ppeimagesize);
    
    FILE *fp = fopen(PE_OUTPUT_FILE, "wb");
    assert(fp);
    ssize_t sret = fwrite(pe, 1, peptr, fp);
    assert(sret == peptr);
    fclose(fp);
    printf("OUTPUT %s\n", PE_OUTPUT_FILE);
    
    return 0;
}
