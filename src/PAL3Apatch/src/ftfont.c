#include "common.h"

#define pd3dDevice (GB_GfxMgr->m_pd3dDevice)

static FT_Library library;

void init_ftfont()
{
    FT_Init_FreeType(&library);
    
    //FT_UInt v = TT_INTERPRETER_VERSION_35;
    //FT_Property_Set(library, "truetype", "interpreter-version", &v);
}



static void ftlayout_clear(struct ftlayout *l, int w, int h, int m)
{
    *l = (struct ftlayout) {
        .w = w,
        .h = h,
        .m = m,
        .u = m,
        .v = m,
        .vnext = m,
    };
}

static void ftlayout_setfull(struct ftlayout *l)
{
    ftlayout_clear(l, 0, 0, 1);
}

static int ftlayout_addrect(struct ftlayout *l, int w, int h, int *u, int *v)
{
    // return value:
    //  <0 --- impossible, even create a new layout with same width and height
    //  0  --- no space left, need create a new layout
    //  >0 --- successful, coord saved in *u and *v
    
    w += l->m;
    h += l->m;
    
    if (w > l->w - l->m || h > l->h - l->m) {
        // not possible
        return -1;
    }
    if (w > l->w - l->u) {
        // no space left in current line
        // switch to new line
        l->u = l->m;
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
static void ftfont_optimize_size_quality(struct ftfont *font)
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
    ret->texw = ret->texh = FTFONT_TEXTURE_MINSIZE;
    ftlayout_setfull(&ret->texlayout);
    
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
    free(ret);
    return NULL;
}


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
    free(ret);
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
    free(ch);
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
    r = ftlayout_addrect(&font->texlayout, ch->w, ch->h, &u, &v);
    if (r <= 0) {
        while (ch->w + 2 * FTFONT_TEXTURE_MARGIN > font->texw) font->texw *= 2;
        while (ch->h + 2 * FTFONT_TEXTURE_MARGIN > font->texh) font->texh *= 2;

        new_node = malloc(sizeof(struct fttexture));
        if (!new_node) goto fail;
        memset(new_node, 0, sizeof(struct fttexture));
        
        if (FAILED(IDirect3DDevice9_CreateTexture(pd3dDevice, font->texw, font->texh, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &new_tex, NULL))) {
            new_tex = NULL;
            goto fail;
        }
        fill_texture(new_tex, 0x00FFFFFF);
        new_node->tex = new_tex;
        new_node->next = font->texhead;
        font->texhead = new_node;
        ftlayout_clear(&font->texlayout, font->texw, font->texh, FTFONT_TEXTURE_MARGIN);
        r = ftlayout_addrect(&font->texlayout, ch->w, ch->h, &u, &v);
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
    free(new_node);
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
        myID3DXSprite_Draw(sprite, ch->tex->tex, &rc, NULL, &pos, color);
    }
    return adv;
}

void ftfont_draw(struct ftfont *font, const wchar_t *wstr, int left, int top, D3DCOLOR color, ID3DXSprite *sprite)
{
    int nleft;
    for (nleft = left; *wstr; wstr++) {
        if (*wstr == '\n') {
            nleft = left;
            top += font->size * font->face->height / font->face->units_per_EM;
        } else if (*wstr != '\r') {
            nleft += ftfont_draw_char(font, *wstr, nleft, top, color, sprite);
        }
    }
}
