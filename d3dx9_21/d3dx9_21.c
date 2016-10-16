#include <windows.h>
#include <d3dx9.h>

/*
	this project will build D3DX9_21.DLL from d3dx9.lib
	the DirectX Summer 2003 SDK should be installed
*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	/* dll will failed to load if SDK has wrong D3DX_SDK_VERSION */
    return D3DX_SDK_VERSION == 21;
}
