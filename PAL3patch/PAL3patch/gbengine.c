#include <d3d9.h>
#include "common.h"
#include "gbengine.h"

// here are some common functions related to GBENGINE.DLL or PAL3.EXE

void gbGfxManager_D3D_Reset3DEnvironment(struct gbGfxManager_D3D *this)
{
    void __fastcall (*gbGfxManager_D3D_Reset3DEnvironment_Real)(struct gbGfxManager_D3D *this); // only 1 argument, no need to add dummy
    gbGfxManager_D3D_Reset3DEnvironment_Real = TOPTR(gboffset + 0x1001AC50);
    gbGfxManager_D3D_Reset3DEnvironment_Real(this);
}


// make sure cooperative level is D3D_OK
// this is my own method! not exists in original GBENGINE
void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus)
{
    if (requirefocus && !is_window_active) {
        while (1) {
            MSG msg;
            // we must process message queue here
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (is_window_active) {
                break;
            }
            Sleep(100);
            call_gameloop_hooks(GAMELOOP_SLEEP);
        }
    }
    if (IDirect3DDevice9_TestCooperativeLevel(this->m_pd3dDevice) != D3D_OK) {
        this->m_bDeviceLost = 1;
        while (1) {
            MSG msg;
            // we must process message queue here
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (IDirect3DDevice9_TestCooperativeLevel(this->m_pd3dDevice) == D3DERR_DEVICENOTRESET) {
                break;
            }
            Sleep(20);
            call_gameloop_hooks(GAMELOOP_DEVICELOST);
        }
        gbGfxManager_D3D_Reset3DEnvironment(this);
        this->m_bDeviceLost = 0;
    }
}
