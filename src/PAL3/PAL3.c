#include "PAL3patch.h"

#define DLLEXPORT __declspec(dllexport)
DLLEXPORT char forwarder_version[] = PATCH_VERSION_STRING;
DLLEXPORT char forwarder_builton[] = __DATE__ ", " __TIME__;
DLLEXPORT char forwarder_compiler[] = BUILD_COMPILER;

#pragma comment(linker, "/export:_sf_entry,@1")
__declspec(naked) void sf_entry(void)
{
	__asm {
		JMP DWORD PTR [sforce_unpacker_entry]
	}
}

#pragma comment(linker, "/entry:dll_entry")
BOOL WINAPI dll_entry(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}
