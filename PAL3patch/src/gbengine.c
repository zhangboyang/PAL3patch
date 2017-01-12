#include "common.h"

// here are some common functions related to GBENGINE.DLL or PAL3.EXE

// this is my own method! not exists in original GBENGINE
enum gbPixelFmtType gbGfxManager_D3D_GetBackBufferFormat(struct gbGfxManager_D3D *this)
{
    enum gbPixelFmtType type;
    switch (this->m_d3dsdBackBuffer.Format) {
        case D3DFMT_A8R8G8B8: type = GB_PFT_A8R8G8B8; break;
        case D3DFMT_R5G6B5:   type = GB_PFT_R5G6B5;   break;
        case D3DFMT_A1R5G5B5: type = GB_PFT_A1R5G5B5; break;
        case D3DFMT_A4R4G4B4: type = GB_PFT_A4R4G4B4; break;
        case D3DFMT_X8R8G8B8: type = GB_PFT_X8R8G8B8; break;
        case D3DFMT_X1R5G5B5: type = GB_PFT_X1R5G5B5; break;
        case D3DFMT_X4R4G4B4: type = GB_PFT_X4R4G4B4; break;
        default:              type = GB_PFT_R8G8B8;   break;
    }
    return type;
}

// make sure cooperative level is D3D_OK
// this is my own method! not exists in original GBENGINE
void gbGfxManager_D3D_EnsureCooperativeLevel(struct gbGfxManager_D3D *this, int requirefocus)
{
    if (requirefocus && !PAL3_s_bActive) {
        while (1) {
            MSG msg;
            // we must process message queue here
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return;
            }
            if (PAL3_s_bActive) {
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
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return;
            }
            if (IDirect3DDevice9_TestCooperativeLevel(this->m_pd3dDevice) == D3DERR_DEVICENOTRESET) {
                break;
            }
            Sleep(100);
            call_gameloop_hooks(GAMELOOP_DEVICELOST);
        }
        if (gbGfxManager_D3D_Reset3DEnvironment(this) < 0) {
            fail("Reset3DEnvironment error!");
        }
        this->m_bDeviceLost = 0;
    }
}

