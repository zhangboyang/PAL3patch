#include <stdio.h>
#include <windows.h>

/*
    gcc shellcode.c -O2 -o shellcode.exe
*/

struct entry_code_data {
    unsigned entry_addr;               // +0x00
    unsigned entry_orig_datasize;      // +0x04
    unsigned char entry_orig_data[8];  // +0x08
    unsigned GetCurrentProcess;        // +0x10
    unsigned WriteProcessMemory;       // +0x14
    
    unsigned LoadLibrary;              // +0x18
    unsigned GetProcAddress;           // +0x1C
    
    unsigned char dllname[0x30];       // +0x20
    unsigned char dllfunc[0x30];       // +0x50
}; // size 0x80


void doit(struct entry_code_data *data)
{
    char *str;
    void *peb;
    void *base;
    void *eat;
    int *list;
    short out16;
    int i;
    void *(__stdcall *my_LoadLibraryA)(char *);
    void *(__stdcall *my_GetProcAddress)(void *, char *);
    void **my_func;
    int cnt = 0;
    
    __asm__ ("movl %%fs:0x30, %0" : "=r" (peb));
    peb = *(void **)(peb + 0x0c);
    peb = *(void **)(peb + 0x14);
    
    while (1) {
        str = *(void **)(peb + 0x28);
        //printf("%ws\n", str);
        __asm__ ("pushl $0x6c6c642e\n\t" /* create 'kernel32.dll' on stack */
                 "pushl $0x32336c65\n\t"
                 "pushl $0x6e72656b\n\t"
                 "xor %%eax, %%eax\n\t"  /* ax - unicode char */
                 "xor %%ebx, %%ebx\n\t"  /* bl - compare cnt, bh - equal cnt */
            "zby_k32_loop:\t"
                 "cmpw $0, (%0)\n\t"        /* if this is the end of string*/
                 "jz zby_k32_cmp_skip\n\t"  /* skip, only pop stack */
                 "movb (%%esp), %%al\n\t"   /* load char from stack */
                 "cmpw (%0), %%ax\n\t"      /* compare */
                 "je zby_k32_cmp_ok\n\t"
                 "subb $0x20, %%al\n\t"     /* convert to upper case */
                 "cmpw (%0), %%ax\n\t"      /* compare */
                 "jne zby_k32_cmp_end\n\t"
            "zby_k32_cmp_ok:\t"
                 "inc %%bh\n\t"             /* compare OK, increase equ cnt */
            "zby_k32_cmp_end:\t"
                 "inc %0\n\t"               /* increase unicode str ptr */
                 "inc %0\n\t"
            "zby_k32_cmp_skip:\t"
                 "inc %%esp\n\t"            /* pop char */
                 "inc %%bl\n\t"             /* increase loop cnt */
                 "cmpb $12, %%bl\n\t"       /* check if we finished */
                 "jne zby_k32_loop\n\t"
                 : "+r" (str), "=b"(out16) :: "eax", "cc", "memory");
        if (out16 == (12 << 8) + 12)
            break;
        peb = *(void **)(peb);
    }
    
    /* should equal to GetModuleHandle("kernel32.dll") */
    base = *(void **)(peb + 0x10);
    //printf("dllbase=%p\n", base);
    
    eat = *(int *)(*(int *)(base + 0x3c) + base + 0x78) + base;
    //printf("EAT=%p\n", eat);
    
    list = *(int *)(eat + 0x20) + base;
    
    for (i = 0; cnt < 2; i++) {
        str = list[i] + base;
        //printf("%s\n", str);
        __asm__ (
                 "xor %%eax, %%eax\n\t"  /* ah - esp diff, al - char */
                 "xor %%ebx, %%ebx\n\t"  /* bl - compare cnt, bh - equal cnt */
                 "cmpb $76, (%0)\n\t"    /* if first char is 'L' */
                 "je zby_func_ll\n\t"   /* compare with 'LoadLibraryA' */
                 "pushl $0x00007373\n\t" /* create 'GetProcAddress' on stack */
                 "pushl $0x65726464\n\t"
                 "pushl $0x41636f72\n\t"
                 "pushl $0x50746547\n\t"
                 "movb $16, %%ah\n\t"    /* size is 16 */
                 "jmp zby_func_loop\n\t"
            "zby_func_ll:\t"
                 "pushl $0x41797261\n\t" /* create 'LoadLibraryA' on stack */
                 "pushl $0x7262694c\n\t"
                 "pushl $0x64616f4c\n\t"
                 "movb $12, %%ah\n\t"    /* size is 12 */
            "zby_func_loop:\t"
                 "cmpb $0, (%0)\n\t"        /* if this is the end of string*/
                 "jz zby_func_cmp_skip\n\t"  /* skip, only pop stack */
                 "movb (%%esp), %%al\n\t"   /* load char from stack */
                 "cmpb (%0), %%al\n\t"      /* compare */
                 "jne zby_func_cmp_end\n\t"
                 "inc %%bh\n\t"             /* compare OK, increase equ cnt */
            "zby_func_cmp_end:\t"
                 "inc %0\n\t"               /* increase str ptr */
            "zby_func_cmp_skip:\t"
                 "inc %%esp\n\t"            /* pop char */
                 "inc %%bl\n\t"             /* increase loop cnt */
                 "cmpb %%ah, %%bl\n\t"       /* check if we finished */
                 "jne zby_func_loop\n\t"
                 : "+r" (str), "+b"(out16) :: "eax", "cc", "memory");
        
        if (out16 == (12 << 8) + 12)
            my_func = (void *) &my_LoadLibraryA;
        else if (out16 == (14 << 8) + 16)
            my_func = (void *) &my_GetProcAddress;
        else
            continue;
        
        *my_func = *(int *)(*(int *)(eat + 0x1c) + base + (
                     *(unsigned short *)(*(int *)(eat + 0x24) + base + i * 2)
                   ) * 4) + base;
        cnt++;
    }

    
    unsigned aGetCurrentProcess[] = {
        0x43746547,
        0x65727275,
        0x7250746E,
        0x7365636F,
        0x00000073,
    };
    
    unsigned aWriteProcessMemory[] = {
        0x74697257,
        0x6F725065,
        0x73736563,
        0x6F6D654D,
        0x00007972,
    };
    data->GetCurrentProcess = (unsigned) my_GetProcAddress(base, (char *) aGetCurrentProcess);
    data->WriteProcessMemory = (unsigned) my_GetProcAddress(base, (char *) aWriteProcessMemory);
    data->LoadLibrary = (unsigned) my_LoadLibraryA;
    data->GetProcAddress = (unsigned) my_GetProcAddress;
}

int main()
{
    struct entry_code_data data;
    doit(&data);
    
    printf("%-30sshellcode:%08x     real:%08x\n", "GetCurrentProcess", data.GetCurrentProcess, (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetCurrentProcess"));
    printf("%-30sshellcode:%08x     real:%08x\n", "WriteProcessMemory", data.WriteProcessMemory, (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "WriteProcessMemory"));
    printf("%-30sshellcode:%08x     real:%08x\n", "LoadLibrary", data.LoadLibrary, (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"));
    printf("%-30sshellcode:%08x     real:%08x\n", "GetProcAddress", data.GetProcAddress, (unsigned) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProcAddress"));

    
    system("pause");
    return 0;
}
