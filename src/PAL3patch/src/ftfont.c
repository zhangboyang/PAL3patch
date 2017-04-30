#include "common.h"

#define pd3dDevice (GB_GfxMgr->m_pd3dDevice)

static FT_Library library;

void init_ftfont()
{
    FT_Init_FreeType(&library);
    
    //FT_UInt v = TT_INTERPRETER_VERSION_35;
    //FT_Property_Set(library, "truetype", "interpreter-version", &v);
}



static void ftfont_clear_layout(struct ftlayout *l, int w, int h, int full)
{
    *l = (struct ftlayout) {
        .w = w,
        .h = h,
        .u = full ? w : 0,
        .v = full ? h : 0,
        .vnext = full ? h : 0,
    };
}

static int ftfont_do_layout(struct ftlayout *l, int w, int h, int *u, int *v)
{
    // return value:
    //  <0 --- impossible, even create a new layout with same width and height
    //  0  --- no space left, need create a new layout
    //  >0 --- successful, coord saved in *u and *v
    
    if (w > l->w || h > l->h) {
        // not possible
        return -1;
    }
    if (w > l->w - l->u) {
        // no space left in current line
        // switch to new line
        l->u = 0;
        l->v = l->vnext;
    }
    if (h > l->h - l->v) {
        // no room, need a new layout
        return 0;
    }
    
    *u = l->u;
    *v = l->v;
    l->u += w;
    if (l->v + h >= l->vnext) l->vnext = l->v + h;
    return 1;
}



// font face has initialized
// adjust size and quality
void ftfont_optimize_size_quality(struct ftfont *font)
{
    // dirty hack: disable bitmap font for MingLiU > 17px
    if (font->face->family_name && strstr(font->face->family_name, "MingLiU")) {
        if (font->size > 17 && font->quality != FTFONT_NOAA) {
            font->quality = FTFONT_AA;
        }
    }
    
    // choose bitmap size
    if (font->quality != FTFONT_AA) {
        int i;
        int max_fixed_size = 0;
        int lower_fixed_size = 0;
        int upper_fixed_size = INT_MAX;
        for (i = 0; i < font->face->num_fixed_sizes; i++) {
            int cursize = font->face->available_sizes[i].size / 64;
            if (cursize > max_fixed_size) max_fixed_size = cursize; 
            if (cursize <= font->size) {
                if (cursize > lower_fixed_size) lower_fixed_size = cursize;
            }
            if (cursize >= font->size) {
                if (cursize < upper_fixed_size) upper_fixed_size = cursize;
            }
        }
        if (max_fixed_size > 0 && lower_fixed_size > 0 && font->size <= max_fixed_size) {
            if (font->size == upper_fixed_size - 1) {
                font->size = upper_fixed_size;
            } else {
                font->size = lower_fixed_size;
            }
            font->quality = FTFONT_NOAA;
        }
    }
}

struct ftfont *ftfont_create(const char *filename, int face_index, int req_size, int req_bold, int req_quality)
{
    struct ftfont *ret = NULL;
    FT_Face face = NULL;
    FT_Error e;
    
    if (!library) goto fail;
    
    // alloc memory
    ret = malloc(sizeof(struct ftfont));
    if (!ret) goto fail;
    memset(ret, 0, sizeof(struct ftfont));
    
    // create face object
    e = FT_New_Face(library, filename, face_index, &face);
    if (e) goto fail;
    ret->face = face;
    ret->size = req_size;
    ret->bold = req_bold;
    ret->quality = req_quality;
    
    // choose better size and quality
    ftfont_optimize_size_quality(ret);

    // set char size
    e = FT_Set_Pixel_Sizes(face, 0, ret->size);
    if (e) goto fail;
    
    // calc texture size and set layout to full
    ret->texw = ret->texh = FTFONT_MIN_TEXTURE_SIZE;
    ftfont_clear_layout(&ret->texlayout, 0, 0, 1);
    
    // set shift data
    ret->yshift = ret->size * face->descender / face->units_per_EM;

    // check if using bitmap font
    if (ret->quality != FTFONT_AA) {
        e = FT_Load_Char(face, FTFONT_BITMAP_TEST_CHAR, FT_LOAD_DEFAULT);
        if (!e) {
            if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                ret->quality = FTFONT_NOAA;
            }
        }
    }
    
    return ret;
fail:
    if (face) FT_Done_Face(face);
    if (ret) free(ret);
    return NULL;
}


struct ftcharhack_bitmap {
    int w, h;
    int yshift;
    const char *data;
};
struct ftcharhack {
    const char *fontname;
    int size;
    const wchar_t *charmap;
    struct ftcharhack_bitmap bitmap[];
};

static struct ftcharhack simsun_12 = {
    "SimSun", 12, L"0123456789/", {
        { 6, 12, -2, "000000000000011100100010100010100010100010100010100010011100000000000000" },
        { 6, 12, -2, "000000000000001000011000001000001000001000001000001000011100000000000000" },
        { 6, 12, -2, "000000000000011100100010100010000100001000010000100000111110000000000000" },
        { 6, 12, -2, "000000000000011100100010000010001100000010000010100010011100000000000000" },
        { 6, 12, -2, "000000000000000100001100010100010100100100011110000100000110000000000000" },
        { 6, 12, -2, "000000000000111110100000100000111100000010000010100010011100000000000000" },
        { 6, 12, -2, "000000000000011100100100100000111100100010100010100010011100000000000000" },
        { 6, 12, -2, "000000000000111110100100000100001000001000001000001000001000000000000000" },
        { 6, 12, -2, "000000000000011100100010100010011100100010100010100010011100000000000000" },
        { 6, 12, -2, "000000000000011100100010100010100010011110000010010010011100000000000000" },
        { 6, 12, -2, "000000000010000100000100000100001000001000010000010000010000100000000000" },
    }
};
static struct ftcharhack simsun_13 = {
    "SimSun", 13, L"0123456789/", {
        { 7, 13, -2, "0000000000000000111000100010010001001000100100010010001001000100100010001110000000000000000" },
        { 7, 13, -2, "0000000000000000010000011000000100000010000001000000100000010000001000001110000000000000000" },
        { 7, 13, -2, "0000000000000000111000100010010001000000100000100000100000100000100000011111000000000000000" },
        { 7, 13, -2, "0000000000000000111000100010000001000011000000010000001000000100100010001110000000000000000" },
        { 7, 13, -2, "0000000000000000001000001100001010001001000100100100010001111000000100000111000000000000000" },
        { 7, 13, -2, "0000000000000001111100100000010000001111000100010000001001000100100010001110000000000000000" },
        { 7, 13, -2, "0000000000000000011100010010010000001011000110010010001001000100100010001110000000000000000" },
        { 7, 13, -2, "0000000000000001111100100100000010000001000001000000100000010000001000000100000000000000000" },
        { 7, 13, -2, "0000000000000000111000100010010001000101000011100010001001000100100010001110000000000000000" },
        { 7, 13, -2, "0000000000000000111000100010010001001000100100110001101000000100100100011100000000000000000" },
        { 7, 13, -2, "0000000000001000001000000100000100000010000010000001000001000000100000100000010000000000000" },
    }
};
static struct ftcharhack simsun_14 = {
    "SimSun", 14, L"0123456789/", {
        { 7, 14, -2, "00000000000000000000000110000100100100001010000101000010100001010000100100100001100000000000000000" },
        { 7, 14, -2, "00000000000000000000000010000111000000100000010000001000000100000010000001000011111000000000000000" },
        { 7, 14, -2, "00000000000000000000000111000100010010001000000100000100000100000100000100010011111000000000000000" },
        { 7, 14, -2, "00000000000000000000000111000100010010001000011000000010000001001000100100010001110000000000000000" },
        { 7, 14, -2, "00000000000000000010000001000001100001010001001000100100100010001111100000100000111000000000000000" },
        { 7, 14, -2, "00000000000000000000001111100100000010000001111000100010000001001000100100010001110000000000000000" },
        { 7, 14, -2, "00000000000000000000000111100100010100000010111001100010100001010000101000010011110000000000000000" },
        { 7, 14, -2, "00000000000000000000001111100100010010010000001000001000000100000010000001000000100000000000000000" },
        { 7, 14, -2, "00000000000000000000001111001000010100001001001000111100100001010000101000010011110000000000000000" },
        { 7, 14, -2, "00000000000000000000001111001000010100001010000101000110011101000000101000100111100000000000000000" },
        { 7, 14, -2, "00000000000001000001000000100000010000010000001000001000000100000100000010000001000001000000000000" },
    }
};
static struct ftcharhack simsun_15 = {
    "SimSun", 15, L"0123456789/", {
        { 8, 15, -2, "000000000000000000000000000110000010010001000010010000100100001001000010010000100100001000100100000110000000000000000000" },
        { 8, 15, -2, "000000000000000000000000000100000111000000010000000100000001000000010000000100000001000000010000011111000000000000000000" },
        { 8, 15, -2, "000000000000000000000000001111000100001001000010000000100000010000001000000100000010000001000010011111100000000000000000" },
        { 8, 15, -2, "000000000000000000000000001111000100001001000010000001000001100000000100000000100100001001000010001111000000000000000000" },
        { 8, 15, -2, "000000000000000000000000000010000000100000011000001010000100100001001000011111100000100000001000000111100000000000000000" },
        { 8, 15, -2, "000000000000000000000000011111100100000001000000010111000110001000000010000000100100001001000010001111000000000000000000" },
        { 8, 15, -2, "000000000000000000000000000111000010010001000000010000000101110001100010010000100100001001000010001111000000000000000000" },
        { 8, 15, -2, "000000000000000000000000011111100100010001000100000010000000100000010000000100000001000000010000000100000000000000000000" },
        { 8, 15, -2, "000000000000000000000000001111000100001001000010010000100011110000100100010000100100001001000010001111000000000000000000" },
        { 8, 15, -2, "000000000000000000000000001110000100010001000010010000100100011000111010000000100000001000100100001110000000000000000000" },
        { 8, 15, -2, "000000000000001000000100000001000000010000001000000010000001000000010000000100000010000000100000010000000100000000000000" },
    }
};
static struct ftcharhack simsun_16 = {
    "SimSun", 16, L"0123456789/", {
        { 8, 16, -2, "00000000000000000000000000011000001001000100001001000010010000100100001001000010010000100100001000100100000110000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000010000011100000001000000010000000100000001000000010000000100000001000000010000011111000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000111100010000100100001001000010000001000000010000001000000100000010000001000010011111100000000000000000" },
        { 8, 16, -2, "00000000000000000000000000111100010000100100001000000100000110000000010000000010000000100100001001000100001110000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000000100000011000001010000100100001001000100010001000100011111100000010000000100000111100000000000000000" },
        { 8, 16, -2, "00000000000000000000000001111110010000000100000001000000010110000110010000000010000000100100001001000100001110000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000011100001001000100000001000000010110000110010001000010010000100100001000100100000110000000000000000000" },
        { 8, 16, -2, "00000000000000000000000001111110010001000100010000001000000010000001000000010000000100000001000000010000000100000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000111100010000100100001001000010001001000001100000100100010000100100001001000010001111000000000000000000" },
        { 8, 16, -2, "00000000000000000000000000011000001001000100001001000010010000100010011000011010000000100000001000100100001110000000000000000000" },
        { 8, 16, -2, "00000000000000000000000100000010000000100000010000000100000010000000100000010000000100000010000000100000010000000100000000000000" },
    }
};
static struct ftcharhack simsun_17 = {
    "SimSun", 17, L"0123456789/", {
        { 9, 17, -2, "000000000000000000000000000000000000000111000001000100010000010010000010010000010010000010010000010010000010010000010001000100000111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000000010000001110000000010000000010000000010000000010000000010000000010000000010000000010000001111100000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000001111000010000100010000100010000100000001000000001000000010000000100000001000000010000100011111100000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000001111000010000100010000100000001000000110000000001000000000100000000100010000100010000100001111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000001000000001000000011000000101000001001000001001000010001000010001000001111100000001000000001000000111110000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000011111100010000000010000000010000000010111000011000100000000100000000100010000100010000100001111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000000111100001000100010000000010000000010111000011000100010000010010000010010000010001000100000111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000011111110010000100010001000000001000000010000000010000000100000000100000000100000000100000000100000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000000111000001000100010000010010000010001000100000111000001000100010000010010000010001000100000111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000000000000000000111000001000100010000010010000010010000010001000110000111010000000010000000010001000100001111000000000000000000000" },
        { 9, 17, -2, "000000000000000000000000010000000100000000100000001100000001000000001000000010000000010000000100000000100000001100000001000000001000000010000000000000000" },
    }
};
static const struct ftcharhack *charhack[] = {
    &simsun_12,
    &simsun_13,
    &simsun_14,
    &simsun_15,
    &simsun_16,
    &simsun_17,
    NULL // EOF
};


static struct ftchar *ftfont_charhack(struct ftfont *font, wchar_t c)
{
    struct ftchar *ret = NULL;
    
    // get font name
    const char *fontname = font->face->family_name;
    if (!fontname) goto fail;
    
    // check quality
    if (font->quality == FTFONT_AA) goto fail;
    
    // lookup for hacks
    const struct ftcharhack **p;
    for (p = charhack; *p; p++) {
        const wchar_t *f;
        if (font->size == (*p)->size && strstr(fontname, (*p)->fontname) && (f = wcschr((*p)->charmap, c))) {
            const struct ftcharhack_bitmap *bitmap = &(*p)->bitmap[f - (*p)->charmap];
            assert((int) strlen(bitmap->data) == bitmap->w * bitmap->h);
            int should_embolden = font->bold >= FTFONT_BITMAP_BOLD_LIMIT;
            ret = malloc(sizeof(struct ftchar) + (bitmap->w + should_embolden) * bitmap->h);
            if (!ret) goto fail;
            ret->tex = NULL;
            ret->u = ret->v = 0;
            ret->w = bitmap->w + should_embolden;
            ret->h = bitmap->h;
            ret->l = 0;
            ret->t = bitmap->h + bitmap->yshift;
            ret->adv = bitmap->w;
            const char *src = bitmap->data;
            unsigned char *dst = ret->bitmap;
            if (should_embolden) {
                int i, j;
                for (i = 0; i < bitmap->h; i++) {
                    unsigned char *dst_line = dst;
                    *dst++ = 0;
                    for (j = 0; j < bitmap->w; j++) {
                        *dst++ = *src++ == '1' ? 0xFF : 0;
                    }
                    for (j = 0; j < bitmap->w; j++) {
                        *dst_line |= *(dst_line + 1);
                        dst_line++;
                    }
                }
            } else {
                while (*src) *dst++ = *src++ == '1' ? 0xFF : 0;
            }
            break;
        }
    }
    
    return ret;
fail:
    if (ret) free(ret);
    return NULL;
}


static void ftfont_loadchar(struct ftfont *font, wchar_t c)
{
    FT_GlyphSlot slot = font->face->glyph;
    FT_Error e;
    struct ftchar *ch = NULL;
    int w, bw;
    int h, bh;
    int i, j;
    int should_embolden_bitmap = 0;
    FT_Int32 load_flags = FT_LOAD_DEFAULT;
    FT_Render_Mode render_mode = FT_RENDER_MODE_NORMAL;
    FT_Bitmap bmp;
    
    // check if already loaded
    if (font->ch[c]) return;
    
    // check if there is a hack
    font->ch[c] = ftfont_charhack(font, c);
    if (font->ch[c]) return;
    
    // process quality setting
    switch (font->quality) {
        case FTFONT_NOAA: load_flags = FT_LOAD_TARGET_MONO; render_mode = FT_RENDER_MODE_MONO; break;
        case FTFONT_AA: load_flags = FT_LOAD_NO_BITMAP; break;
        case FTFONT_AUTO: break;
    }
    
    // load char with freetype
    e = FT_Load_Char(font->face, c, load_flags);
    if (e) goto fail;
    
    // embolden outline if needed
    if (font->bold) {
        if (slot->format == FT_GLYPH_FORMAT_BITMAP || font->quality == FTFONT_NOAA) {
            if (font->bold >= FTFONT_BITMAP_BOLD_LIMIT) {
                should_embolden_bitmap = 1;
            }
        } else if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
            FT_Outline_Embolden(&slot->outline, font->bold);
        }
    }
    
    // render char
    e = FT_Render_Glyph(slot, render_mode);
    if (e) goto fail;
    
    // convert bitmap
    FT_Bitmap_Init(&bmp);
    e = FT_Bitmap_Convert(library, &slot->bitmap, &bmp, 1);
    if (e) goto bmpfail; 
    
    // embolden bitmap if needed
    if (should_embolden_bitmap) {
        FT_Bitmap_Embolden(library, &bmp, 64, 0);
    }
    
    // get width and height of current char
    w = bw = bmp.width;
    h = bh = bmp.rows;
    if (w == 0) w++;
    if (h == 0) h++;
    
    // alloc memory
    ch = malloc(sizeof(struct ftchar) + w * h);
    if (!ch) goto bmpfail;
    memset(ch, 0, sizeof(struct ftchar) + w * h);
    
    ch->tex = NULL;
    ch->u = ch->v = 0;
    ch->w = w;
    ch->h = h;
    ch->l = slot->bitmap_left;
    ch->t = slot->bitmap_top;
    ch->adv = round(slot->advance.x / 64.0);
    
    // copy bitmap
    for (i = 0; i < bh; i++) {
        for (j = 0; j < bw; j++) {
            ch->bitmap[w * i + j] = bmp.buffer[bmp.pitch * i + j] * 255 / (bmp.num_grays - 1);
        }
    }

    font->ch[c] = ch;
    ch = NULL;
bmpfail:
    FT_Bitmap_Done(library, &bmp);
fail:
    if (ch) free(ch);
}

void ftfont_preload_range(struct ftfont *font, wchar_t low, wchar_t high)
{
    wchar_t c;
    for (c = low; c <= high; c++) {
        ftfont_loadchar(font, c);
    }
}
void ftfont_preload_string(struct ftfont *font, const wchar_t *wstr)
{
    while (*wstr) {
        ftfont_preload_range(font, *wstr, *wstr);
        wstr++;
    }
}




static void ftfont_assign_texture(struct ftfont *font, wchar_t c)
{
    struct fttexture *new_node = NULL;
    IDirect3DTexture9 *new_tex = NULL;
    RECT rc;
    D3DLOCKED_RECT lrc;
    int r, u, v;
    
    // load char first
    ftfont_loadchar(font, c);
    struct ftchar *ch = font->ch[c];
    if (!ch) return;
    if (ch->tex) return;
    
    // layout char
    r = ftfont_do_layout(&font->texlayout, ch->w, ch->h, &u, &v);
    if (r <= 0) {
        while (ch->w > font->texw) font->texw *= 2;
        while (ch->h > font->texh) font->texh *= 2;

        new_node = malloc(sizeof(struct fttexture));
        if (!new_node) goto fail;
        memset(new_node, 0, sizeof(struct fttexture));
        
        if (FAILED(IDirect3DDevice9_CreateTexture(pd3dDevice, font->texw, font->texh, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &new_tex, NULL))) {
            new_tex = NULL;
            goto fail;
        }
        fill_texture(new_tex, 0x00000000);
        new_node->tex = new_tex;
        new_node->next = font->texhead;
        font->texhead = new_node;
        ftfont_clear_layout(&font->texlayout, font->texw, font->texh, 0);
        r = ftfont_do_layout(&font->texlayout, ch->w, ch->h, &u, &v);
        assert(r > 0);
    }
    ch->tex = font->texhead;
    ch->u = u;
    ch->v = v;
    
    // copy to texture
    set_rect_ltwh(&rc, ch->u, ch->v, ch->w, ch->h);
    if (SUCCEEDED(IDirect3DTexture9_LockRect(ch->tex->tex, 0, &lrc, &rc, 0))) {
        int i, j;
        for (i = 0; i < ch->h; i++) {
            for (j = 0; j < ch->w; j++) {
                unsigned char *pt = PTRADD(lrc.pBits, i * lrc.Pitch + j * 4);
                pt[0] = pt[1] = pt[2] = 0xff;
                pt[3] = ch->bitmap[i * ch->w + j];
            }
        }
        IDirect3DTexture9_UnlockRect(ch->tex->tex, 0);
    }
    
    return;
fail:
    if (new_node) free(new_node);
    if (new_tex) IDirect3DTexture9_Release(new_tex);
}

static int ftfont_draw_char(struct ftfont *font, wchar_t c, int left, int top, D3DCOLOR color, ID3DXSprite *sprite)
{
    int adv = font->size;
    ftfont_assign_texture(font, c);
    struct ftchar *ch = font->ch[c];
    top += font->size + font->yshift;
    left += font->xshift;
    if (ch && ch->tex) {
        adv = ch->adv;
        RECT rc;
        set_rect_ltwh(&rc, ch->u, ch->v, ch->w, ch->h);
        left += ch->l;
        top -= ch->t;
        D3DXVECTOR3 pos = { left, top, 0.0f };
        ID3DXSprite_Draw(sprite, ch->tex->tex, &rc, NULL, &pos, color);
    }
    return adv;
}

void ftfont_draw(struct ftfont *font, const wchar_t *wstr, int left, int top, D3DCOLOR color, ID3DXSprite *sprite)
{
    while (*wstr) {
        int adv = ftfont_draw_char(font, *wstr, left, top, color, sprite);
        wstr++;
        left += adv;
    }
}
