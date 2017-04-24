#include "common.h"

#define pd3dDevice (GB_GfxMgr->m_pd3dDevice)

static FT_Library library;

void init_ftfont()
{
    FT_Init_FreeType(&library);
    
    FT_UInt v = TT_INTERPRETER_VERSION_35;
    FT_Property_Set(library, "truetype", "interpreter-version", &v);
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





struct ftfont *ftfont_create(const char *filename, int face_index, int size, int bold, int quality)
{
    struct ftfont *ret = NULL;
    FT_Face face = NULL;
    FT_Error e;
    
    if (!library) goto fail;
    
    // alloc memory
    ret = malloc(sizeof(struct ftfont));
    if (!ret) goto fail;
    memset(ret, 0, sizeof(struct ftfont));
    
    // calc texture size and set layout to full
    for (ret->texsize = 64; ret->texsize < size; ret->texsize *= 2);
    ftfont_clear_layout(&ret->texlayout, ret->texsize, ret->texsize, 1);
    
    // create face object
    e = FT_New_Face(library, filename, face_index, &face);
    if (e) goto fail;
    ret->face = face;
    
    // choose bitmap size
    if (quality != FTFONT_AA) {
        int i;
        int max_fixed_size = 0;
        int lower_fixed_size = 0;
        int upper_fixed_size = INT_MAX;
        for (i = 0; i < face->num_fixed_sizes; i++) {
            int cursize = face->available_sizes[i].size / 64;
            if (cursize > max_fixed_size) max_fixed_size = cursize; 
            if (cursize <= size) {
                if (cursize > lower_fixed_size) lower_fixed_size = cursize;
            }
            if (cursize >= size) {
                if (cursize < upper_fixed_size) upper_fixed_size = cursize;
            }
        }
        if (max_fixed_size > 0) {
            if (size == upper_fixed_size - 1) {
                size = upper_fixed_size;
            } else if (size == lower_fixed_size + 1) {
                size = lower_fixed_size;
            }
        }
        if (size <= max_fixed_size) {
            quality = FTFONT_NOAA;
        }
    }
    
    // set char size
    e = FT_Set_Pixel_Sizes(face, 0, size);
    if (e) goto fail;
    
    // check if using bitmap font
    if (quality != FTFONT_AA) {
        e = FT_Load_Char(face, FTFONT_BITMAP_TEST_CHAR, FT_LOAD_DEFAULT);
        if (!e) {
            if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                quality = FTFONT_NOAA;
            }
        }
    }
    
    ret->size = size;
    ret->bold = bold;
    ret->quality = quality;
    
    return ret;
fail:
    if (face) FT_Done_Face(face);
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
    if (r < 0) return;
    if (r == 0) {
        new_node = malloc(sizeof(struct fttexture));
        if (!new_node) goto fail;
        if (FAILED(IDirect3DDevice9_CreateTexture(pd3dDevice, font->texsize, font->texsize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &new_tex, NULL))) {
            new_tex = NULL;
            goto fail;
        }
        new_node->tex = new_tex;
        new_node->next = font->texhead;
        font->texhead = new_node;
        ftfont_clear_layout(&font->texlayout, font->texsize, font->texsize, 0);
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
    top += font->size;
    if (ch && ch->tex) {
        adv = ch->adv;
        RECT rc;
        set_rect_ltwh(&rc, ch->u, ch->v, ch->w, ch->h);
        left += ch->l;
        top -= ch->t;
        D3DXVECTOR3 pos;
        pos.x = left;
        pos.y = top;
        pos.z = 0.0f;
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
