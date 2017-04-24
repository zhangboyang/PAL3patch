#ifndef PAL3PATCH_FTFONT_H
#define PAL3PATCH_FTFONT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_DRIVER_H
#include FT_MODULE_H
#include FT_BITMAP_H


#define FTFONT_MAXCHARS 0x10000
#define FTFONT_BITMAP_BOLD_LIMIT 64
#define FTFONT_BITMAP_TEST_CHAR L'\x6587'

enum ftquality {
    FTFONT_NOAA,
    FTFONT_AA,
    FTFONT_AUTO,
};
struct ftlayout {
    int w, h;
    int u, v, vnext;
};

struct fttexture {
    IDirect3DTexture9 *tex;
    struct fttexture *next;
};

struct ftchar {
    struct fttexture *tex;
    int u, v;
    int w, h;
    int l, t, adv;
    unsigned char bitmap[];
};

struct ftfont {
    FT_Face face;
    
    int size;
    int bold;
    int quality;
    int texsize;
    
    struct fttexture *texhead;
    struct ftlayout texlayout;

    struct ftchar *ch[FTFONT_MAXCHARS];
};

extern void init_ftfont();
extern struct ftfont *ftfont_create(const char *filename, int face_index, int size, int bold, int quality);
extern void ftfont_preload_range(struct ftfont *font, wchar_t low, wchar_t high);
extern void ftfont_preload_string(struct ftfont *font, const wchar_t *wstr);
extern void ftfont_draw(struct ftfont *font, const wchar_t *wstr, int left, int top, D3DCOLOR color, ID3DXSprite *sprite);

#endif
