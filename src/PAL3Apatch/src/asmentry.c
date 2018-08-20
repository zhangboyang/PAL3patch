struct trapframe;
extern void patchentry(struct trapframe *tf);

#define MAX_PUSH_DWORDS 16

#define X ((MAX_PUSH_DWORDS - 3) * 4)
#define Y ((MAX_PUSH_DWORDS - 1) * 4)
#define Z ((MAX_PUSH_DWORDS + 3) * 4)

unsigned max_push_dwords = MAX_PUSH_DWORDS;

__declspec(naked) void __stdcall asmentry(unsigned patch_id)
{
    __asm {
        PUSHFD
        SUB ESP, X
        PUSH DWORD PTR [ESP + Y]
        PUSH DWORD PTR [ESP + Y]
        PUSH DWORD PTR [ESP + Y]
        PUSHAD
        ADD DWORD PTR [ESP + 0xC], Z
        SUB ESP, 0x6C
        FSAVE [ESP]
        PUSH ESP
        CALL patchentry
        ADD ESP, 0x4
        FRSTOR [ESP]
        ADD ESP, 0x6C
        MOV ECX, DWORD PTR [ESP + 0xC]
        MOV EAX, DWORD PTR [ESP + 0x24]
        MOV EDX, DWORD PTR [ESP + 0x20]
        SUB ECX, 0x8
        MOV DWORD PTR [ECX + 0x4], EAX
        MOV DWORD PTR [ECX], EDX
        MOV DWORD PTR [ESP + 0x20], ECX
        POPAD
        POP ESP
        POPFD
        RET
    }
}
