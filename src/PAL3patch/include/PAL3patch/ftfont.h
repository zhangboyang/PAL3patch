#ifndef PAL3PATCH_FTFONT_H
#define PAL3PATCH_FTFONT_H
// PATCHAPI DEFINITIONS

struct ftfont;

extern PATCHAPI struct ftfont *ftfont_create(const char *filename, int face_index, int req_size, int req_bold, int req_quality);
extern PATCHAPI void ftfont_preload_range(struct ftfont *font, wchar_t low, wchar_t high);
extern PATCHAPI void ftfont_preload_string(struct ftfont *font, const wchar_t *wstr);
extern PATCHAPI void ftfont_draw(struct ftfont *font, const wchar_t *wstr, int left, int top, D3DCOLOR color, ID3DXSprite *sprite);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#if FREETYPE_PATCH != 0
#define FTFONT_VERSTR TOSTR(FREETYPE_MAJOR) TOSTR(FREETYPE_MINOR) TOSTR(FREETYPE_PATCH)
#else
#define FTFONT_VERSTR TOSTR(FREETYPE_MAJOR) TOSTR(FREETYPE_MINOR)
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

extern void init_ftfont(void);

#endif
#endif
