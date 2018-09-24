#include "PAL3Apatch.h"
#pragma comment(lib, "PAL3Apatch.lib")

#define DLLEXPORT __declspec(dllexport)
#pragma comment(linker, "/export:_sf_entry,@1")

DLLEXPORT const char forwarder_version[] = PATCH_VERSION_STRING;
DLLEXPORT const char forwarder_builton[] = __DATE__ ", " __TIME__;
DLLEXPORT const char forwarder_compiler[] = BUILD_COMPILER;

__declspec(naked) void sf_entry(void)
{
	__asm {
		JMP DWORD PTR [sforce_unpacker_entry]
	}
}
