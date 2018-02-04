#pragma comment(linker, "/export:_sforce_unpacker_entry")

extern unsigned sforce_unpacker_init(void);

__declspec(naked) void sforce_unpacker_entry(void)
{
    __asm {
        SUB ESP, 4
        PUSHAD
        CALL sforce_unpacker_init
        MOV [ESP + 0x20], EAX
        POPAD
        RET
    }
}
