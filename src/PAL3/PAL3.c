#pragma comment(lib, "PAL3patch.lib")
#pragma comment(linker, "/export:_entry_stub,@1,NONAME")

extern __declspec(dllimport) void sforce_unpacker_entry(void);

__declspec(naked) void entry_stub(void)
{
	__asm {
		JMP DWORD PTR [sforce_unpacker_entry]
	}
}