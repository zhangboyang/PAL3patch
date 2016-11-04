#include "common.h"


// replace font using D3DXFont
enum {
    D3DXFONT_U12, // UNICODE 12
    D3DXFONT_U16, // the real size of U16 might be 14px
    D3DXFONT_U20,

    D3DXFONT_COUNT // EOF
};
#define D3DXFONT_MAXFONTS (D3DXFONT_COUNT * (SCALEFACTOR_COUNT + 1))

static DWORD d3dxfont_charset;
static DWORD d3dxfont_quality;
static LPWSTR d3dxfont_facename;
static int d3dxfont_sizelist_orig[D3DXFONT_COUNT] = {12, 16, 20};
static int d3dxfont_sizelist[D3DXFONT_COUNT];
static int d3dxfont_boldflag[D3DXFONT_COUNT];

struct d3dxfont_desc {
    int fontsize;
    int boldflag;
    ID3DXFont *pfont;
};
static struct d3dxfont_desc d3dxfont_fontlist[D3DXFONT_MAXFONTS];
static int d3dxfont_fontcnt = 0;

static ID3DXSprite *d3dxfont_sprite = NULL;
static IDirect3DStateBlock9 *d3dxfont_stateblock = NULL;

static int d3dxfont_desc_cmp(const void *a, const void *b)
{
    const struct d3dxfont_desc *pa = a, *pb = b;
    if (pa->fontsize != pb->fontsize) return pa->fontsize < pb->fontsize ? -1 : 1;
    if (pa->boldflag != pb->boldflag) return pa->boldflag < pb->boldflag ? -1 : 1;
    return 0;
}

static int d3dxfont_getfontid_orig(int fontsize_orig)
{
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        if (d3dxfont_sizelist_orig[i] == fontsize_orig) return i;
    }
    return D3DXFONT_U16;
}
static int d3dxfont_selectbysize(int fontsize_orig)
{
    int fontid_orig = d3dxfont_getfontid_orig(fontsize_orig);
    
    struct d3dxfont_desc key = {
        .fontsize = fs->len_factor * d3dxfont_sizelist[fontid_orig],
        .boldflag = d3dxfont_boldflag[fontid_orig],
        .pfont = NULL,
    };
    
    struct d3dxfont_desc *ptr = bsearch(&key, d3dxfont_fontlist, d3dxfont_fontcnt, sizeof(struct d3dxfont_desc), d3dxfont_desc_cmp);
    if (!ptr) {
        // font not found, this should not happen, fallback to original size
        warning("font not found for size %d, bold %d.", key.fontsize, key.boldflag);
        key = (struct d3dxfont_desc) {
            .fontsize = d3dxfont_sizelist[fontid_orig],
            .boldflag = d3dxfont_boldflag[fontid_orig],
            .pfont = NULL,
        };
        ptr = bsearch(&key, d3dxfont_fontlist, d3dxfont_fontcnt, sizeof(struct d3dxfont_desc), d3dxfont_desc_cmp);
        if (!ptr) fail("d3dx font not found.");
    }
    return ptr - d3dxfont_fontlist;
}

static void d3dxfont_insertfont(int fontsize, int boldflag)
{
    if (fontsize <= 0) {
        warning("try insert a font with zero size.");
    }
    
    struct d3dxfont_desc key = {
        .fontsize = fontsize,
        .boldflag = boldflag,
        .pfont = NULL,
    };
    if (bsearch(&key, d3dxfont_fontlist, d3dxfont_fontcnt, sizeof(struct d3dxfont_desc), d3dxfont_desc_cmp) != NULL) {
        // font already exists, no need to create and insert
        return;
    }
    
    // create the font
    if (FAILED(D3DXCreateFontW(g_GfxMgr->m_pd3dDevice, fontsize, 0, (boldflag ? FW_BOLD : 0), 0, FALSE, d3dxfont_charset, OUT_DEFAULT_PRECIS, d3dxfont_quality, DEFAULT_PITCH | FF_DONTCARE, d3dxfont_facename, &key.pfont))) {
        fail("can't create ID3DXFont for size '%d'.", fontsize);
    }
    
    // insert to list
    if (d3dxfont_fontcnt >= D3DXFONT_MAXFONTS) fail("too many d3dx fonts.");
    d3dxfont_fontlist[d3dxfont_fontcnt++] = key;
    qsort(d3dxfont_fontlist, d3dxfont_fontcnt, sizeof(struct d3dxfont_desc), d3dxfont_desc_cmp);
}

static void d3dxfont_init()
{
    // the init function must called after IDirect3DDevice is initialized
    // the scalefactors should be set at this time
    
    // create fonts, O(n^2*log(n))
    int i, j; 
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        // original sizes
        d3dxfont_insertfont(d3dxfont_sizelist[i], d3dxfont_boldflag[i]);
    }
    for (i = 0; i < SCALEFACTOR_COUNT; i++) {
        for (j = 0; j < D3DXFONT_COUNT; j++) {
            d3dxfont_insertfont(scalefactor_table[i] * d3dxfont_sizelist[j], d3dxfont_boldflag[j]);
        }
    }
    
    if (FAILED(D3DXCreateSprite(g_GfxMgr->m_pd3dDevice, &d3dxfont_sprite))) {
        fail("can't create sprite for font replacing.");
    }
    
    if (FAILED(IDirect3DDevice9_CreateStateBlock(g_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &d3dxfont_stateblock))) {
        fail("can't create state block for font replacing.");
    }
}
static void d3dxfont_printwstr(ID3DXSprite *sprite, int fontid, LPWSTR wstr, int left, int top, D3DCOLOR color)
{
    RECT rc;
    set_rect(&rc, left, top, 0, 0);
    ID3DXFont_DrawTextW(d3dxfont_fontlist[fontid].pfont, sprite, wstr, -1, &rc, DT_NOCLIP, color);
}

static void d3dxfont_onlostdevice()
{
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        ID3DXFont_OnLostDevice(d3dxfont_fontlist[i].pfont);
    }
    ID3DXSprite_OnLostDevice(d3dxfont_sprite);
    IDirect3DStateBlock9_Release(d3dxfont_stateblock);
}
static void d3dxfont_onresetdevice()
{
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        ID3DXFont_OnResetDevice(d3dxfont_fontlist[i].pfont);
    }
    ID3DXSprite_OnResetDevice(d3dxfont_sprite);
    if (FAILED(IDirect3DDevice9_CreateStateBlock(g_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &d3dxfont_stateblock))) {
        fail("can't create state block for font replacing.");
    }
}

struct d3dxfont_strnode {
    int fontid;
    wchar_t *wstr;
    int left;
    int top;
    D3DCOLOR color;
    struct d3dxfont_strnode *next;
};

struct d3dxfont_strnode *d3dxfont_strlist_head = NULL, *d3dxfont_strlist_tail = NULL;

static void __fastcall gbPrintFont_UNICODE_PrintString(struct gbPrintFont_UNICODE *this, int dummy, const char *str, float x, float y, float endx, float endy)
{
    fixui_update_gamestate();
    
    // make a node
    struct d3dxfont_strnode *node = malloc(sizeof(struct d3dxfont_strnode));
    node->fontid = d3dxfont_selectbysize(this->fontsize);
    node->wstr = cs2wcs(str, target_codepage);
    node->color = this->curColor.Color; // FIXME: should we check gbColorQuad::ColorQuadFmt ?

    // calc coord
    fRECT frect;
    set_frect_ltwh(&frect, (x + 1.0) * gfxdrvinfo.width / 2.0, (1.0 - y) * gfxdrvinfo.height / 2.0 - d3dxfont_sizelist_orig[d3dxfont_getfontid_orig(this->fontsize)], 0, 0);
    fixui_adjust_fRECT(&frect, &frect);
    node->left = round(frect.left);
    node->top = round(frect.top);
    

    // append to linked-list
    node->next = NULL;
    if (d3dxfont_strlist_head == NULL || d3dxfont_strlist_tail == NULL) {
        d3dxfont_strlist_head = d3dxfont_strlist_tail = node;
    } else {
        d3dxfont_strlist_tail->next = node;
        d3dxfont_strlist_tail = node;
    }
}

static void __fastcall gbPrintFont_UNICODE_Flush(struct gbPrintFont_UNICODE *this, int dummy)
{
    struct d3dxfont_strnode *node, *nextnode;
    
    // save device state
    IDirect3DStateBlock9_Capture(d3dxfont_stateblock);
    
    // make text in front of other pixels
    IDirect3DDevice9_SetRenderState(g_GfxMgr->m_pd3dDevice, D3DRS_ZFUNC, D3DCMP_ALWAYS);
    
    // draw strings in linked-list
    ID3DXSprite_Begin(d3dxfont_sprite, D3DXSPRITE_ALPHABLEND);
    for (node = d3dxfont_strlist_head; node; node = node->next) {
        d3dxfont_printwstr(d3dxfont_sprite, node->fontid, node->wstr, node->left, node->top, node->color);
    }
    ID3DXSprite_End(d3dxfont_sprite);
    
    // restore device state
    IDirect3DStateBlock9_Apply(d3dxfont_stateblock);
    
    
    // clear the linked-list and free memory
    for (node = d3dxfont_strlist_head; node; node = nextnode) {
        nextnode = node->next;
        free(node->wstr);
        free(node);
    }
    d3dxfont_strlist_head = d3dxfont_strlist_tail = NULL;
}

static void ui_replacefont_d3dxfont_init()
{
    // read configurations
    switch (target_codepage) {
        case 936: d3dxfont_charset = GB2312_CHARSET; break;
        case 950: d3dxfont_charset = CHINESEBIG5_CHARSET; break;
        default:  d3dxfont_charset = DEFAULT_CHARSET; break;
    }
    d3dxfont_quality = get_int_from_configfile("uireplacefont_quality");
    d3dxfont_facename = cs2wcs(get_string_from_configfile("uireplacefont_facename"), CP_ACP); // let it leak
    if (sscanf(get_string_from_configfile("uireplacefont_size"), "%d,%d,%d", &d3dxfont_sizelist[D3DXFONT_U12], &d3dxfont_sizelist[D3DXFONT_U16], &d3dxfont_sizelist[D3DXFONT_U20]) != 3) {
        fail("can't parse font size string.");
    }
    if (sscanf(get_string_from_configfile("uireplacefont_bold"), "%d,%d,%d", &d3dxfont_boldflag[D3DXFONT_U12], &d3dxfont_boldflag[D3DXFONT_U16], &d3dxfont_boldflag[D3DXFONT_U20]) != 3) {
        fail("can't parse font bold flag string.");
    }
    
    // add hooks
    add_postd3dcreate_hook(d3dxfont_init);
    add_onlostdevice_hook(d3dxfont_onlostdevice);
    add_onresetdevice_hook(d3dxfont_onresetdevice);
    
    // hook gbPrintFont_UNICODE's member function
    make_jmp(gboffset + 0x10023A90, gbPrintFont_UNICODE_Flush);
    make_jmp(gboffset + 0x10023FD0, gbPrintFont_UNICODE_PrintString);
}





// replace font using PAL3's own bitmap font
//   pros: no system font needed, works on systems of any language
//   cons: only size 12, 16, 20 is supported

static struct gbPrintFont *get_gbprintfont(int fontsize)
{
    switch (fontsize) { // according to UIDrawText() in PAL3
        case 12: return gbPrintFontMgr_GetFont(g_GfxMgr->pFontMgr, GB_FONT_UNICODE12);
        case 16: return gbPrintFontMgr_GetFont(g_GfxMgr->pFontMgr, GB_FONT_UNICODE16);
        case 20: return gbPrintFontMgr_GetFont(g_GfxMgr->pFontMgr, GB_FONT_UNICODE20);
        case 24: return gbPrintFontMgr_GetFont(g_GfxMgr->pFontMgr, GB_FONT_NUMBER);
        // FIXME: where is GB_FONT_ASC
        default: return NULL;
    }
}

static int map_bitmapfontsize(int fontsize)
{
    if (fontsize != 12 && fontsize != 16 && fontsize != 20) {
        // unknown fontsize, we should return fontsize directly
        return fontsize;
    }
    
    double ideal_fontsize;
    
    ideal_fontsize = fontsize * fs->len_factor;
    
    if (ideal_fontsize >= 20) return 20;
    if (ideal_fontsize >= 16) return 16;
    if (ideal_fontsize >= 12) return 12;
    return 16; // if fontsize is too small, default to 12
}

// hook UIDrawTextEx()
static void UIDrawTextEx_wrapper(const char *str, RECT *rect, struct gbPrintFont *font, int fontsize, int middleflag)
{
    fixui_update_gamestate();
    
    // scale fontsize
    int new_fontsize = map_bitmapfontsize(fontsize);
    double font_scalefactor = (double) new_fontsize / fontsize;
    
    // scale rect using normal method
    fRECT old_frect, new_frect;
    set_frect_rect(&old_frect, rect);
    fixui_adjust_fRECT(&new_frect, &old_frect);
    
    // scale rect by font_scalefactor
    transform_frect(&new_frect, &old_frect, &old_frect, &new_frect, TR_CENTER, TR_CENTER, font_scalefactor);
    
    // call original UIDrawTextEx()
    RECT new_rect;
    set_rect_frect(&new_rect, &new_frect);
    struct gbPrintFont *new_font = get_gbprintfont(new_fontsize);
    if (!new_font) new_font = font;
    new_font->curColor = font->curColor; // copy color
    UIDrawTextEx(str, &new_rect, new_font, new_fontsize, middleflag);
}
static void hook_UIDrawTextEx()
{
    // xref of UIDrawTextEx()
    make_call(0x00450533, UIDrawTextEx_wrapper);
    make_call(0x005411F2, UIDrawTextEx_wrapper);
}

// hook UIPrint()
static void UIPrint_wrapper(int x, int y, char *str, struct gbColorQuad *color, int fontsize)
{
    fixui_update_gamestate();
    RECT tmp_rect;
    set_rect(&tmp_rect, x, y, 0, 0);
    fixui_adjust_RECT(&tmp_rect, &tmp_rect);
    fontsize = map_bitmapfontsize(fontsize);
    // UIPrint() will automaticly select gbPrintFont by fontsize
    UIPrint(tmp_rect.left, tmp_rect.top, str, color, fontsize);
}
static void hook_UIPrint()
{
    unsigned funcptr[] = { // xref of UIPrint()
        0x0043E8A0 + 0x11F,
        0x0043E8A0 + 0x13B,
        0x0044DAC0 + 0x66,
        0x0053C990 + 0xE6,
        0x0053C990 + 0x10A,
        0x0053C990 + 0x143,
        0 // eof
    };
    unsigned *ptr;
    for (ptr = funcptr; *ptr; ptr++) {
        make_call(*ptr, UIPrint_wrapper);
    }
}

static void ui_replacefont_bitmapfont_init()
{
    hook_UIDrawTextEx();
    hook_UIPrint();
}









MAKE_PATCHSET(uireplacefont)
{
    switch (flag) {
        case 1: ui_replacefont_d3dxfont_init(); break;
        case 2: ui_replacefont_bitmapfont_init(); break;
        default: fail("unknown flag %d for uireplacefont.", flag);
    }
}
