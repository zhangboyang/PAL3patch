#ifndef PAL3PATCH_FTFONT_H
#define PAL3PATCH_FTFONT_H
#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_DRIVER_H
#include FT_MODULE_H
#include FT_BITMAP_H


#if FREETYPE_PATCH != 0
#define FTFONT_VERSTR TOSTR(FREETYPE_MAJOR) TOSTR(FREETYPE_MINOR) TOSTR(FREETYPE_PATCH)
#else
#define FTFONT_VERSTR TOSTR(FREETYPE_MAJOR) TOSTR(FREETYPE_MINOR)
#endif


#if defined(_MSC_VER)
#ifdef _DEBUG
#pragma comment(lib, "freetype" FTFONT_VERSTR "d.lib")
#else
#pragma comment(lib, "freetype" FTFONT_VERSTR ".lib")
#endif
#endif



#define FTFONT_MAXCHARS 0x10000
#define FTFONT_BITMAP_BOLD_LIMIT 48
#define FTFONT_BITMAP_TEST_CHAR L'\x6587'
#define FTFONT_TEXTURE_MINSIZE 64
#define FTFONT_TEXTURE_MARGIN 4

enum ftquality {
    FTFONT_NOAA,
    FTFONT_AA,
    FTFONT_AUTO,
};
struct ftlayout {
    int w, h;
    int u, v, vnext;
    int m;
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

    int xshift;
    int yshift;
    
    struct fttexture *texhead;
    int texw;
    int texh;
    struct ftlayout texlayout;

    struct ftchar *ch[FTFONT_MAXCHARS];
};

extern void init_ftfont();
extern struct ftfont *ftfont_create(const char *filename, int face_index, int req_size, int req_bold, int req_quality);
extern void ftfont_preload_range(struct ftfont *font, wchar_t low, wchar_t high);
extern void ftfont_preload_string(struct ftfont *font, const wchar_t *wstr);
extern void ftfont_draw(struct ftfont *font, const wchar_t *wstr, int left, int top, D3DCOLOR color, ID3DXSprite *sprite);

#endif
#endif
