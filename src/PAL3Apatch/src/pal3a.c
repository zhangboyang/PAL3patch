#include "common.h"


void clamp_rect(void *bits, int width, int height, int bitcount, int pitch, int left, int top, int right, int bottom)
{
    int i, j;
    int bytecount = bitcount / 8;
    for (i = 0; i < height; i++) {
        char *line = PTRADD(bits, i * pitch);
        for (j = 0; j < left; j++) {
            memcpy(line + j * bytecount, line + left * bytecount, bytecount);
        }
        for (j = right; j < width; j++) {
            memcpy(line + j * bytecount, line + (right - 1) * bytecount, bytecount);
        }
    }
    for (i = 0; i < top; i++) {
        memcpy(PTRADD(bits, i * pitch), PTRADD(bits, top * pitch), width * bytecount);
    }
    for (i = bottom; i < height; i++) {
        memcpy(PTRADD(bits, i * pitch), PTRADD(bits, (bottom - 1) * pitch), width * bytecount);
    }
}

void copy_bits(void *dst, int dst_pitch, int dst_x, int dst_y, void *src, int src_pitch, int src_x, int src_y, int width, int height, int bitcount)
{
    dst = PTRADD(dst, dst_pitch * dst_y + dst_x * (bitcount / 8));
    src = PTRADD(src, src_pitch * src_y + src_x * (bitcount / 8));
    int copypitch = width * (bitcount / 8);
    int i;
    for (i = 0; i < height; i++) {
        memcpy(dst, src, copypitch);
        dst = PTRADD(dst, dst_pitch);
        src = PTRADD(src, src_pitch);
    }
}

// setup matrices for d3dxfont manually
void set_d3dxfont_matrices(IDirect3DDevice9 *pd3dDevice)
{
    float world[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };
    float view[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f },
    };
    float projection[4][4] = {
		{ 2.0f / get_frect_width(&game_frect), 0.0f, 0.0f, 0.0f },
		{ 0.0f, -2.0f / get_frect_height(&game_frect), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
    };
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_WORLD, (D3DMATRIX *) &world);
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_VIEW, (D3DMATRIX *) &view);
    IDirect3DDevice9_SetTransform(pd3dDevice, D3DTS_PROJECTION, (D3DMATRIX *) &projection);
}


// fill texture with given color
static VOID WINAPI fill_texture_callback(D3DXVECTOR4 *pOut, const D3DXVECTOR2 *pTexCoord, const D3DXVECTOR2 *pTexelSize, LPVOID pData)
{
   *pOut = *(D3DXVECTOR4 *) pData;
}
void fill_texture(IDirect3DTexture9 *tex, D3DCOLOR color)
{
    float r = (color & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = ((color >> 16) & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    D3DXVECTOR4 vcolor = (D3DXVECTOR4) { r, g, b, a };
    D3DXFillTexture(tex, fill_texture_callback, &vcolor);
}
