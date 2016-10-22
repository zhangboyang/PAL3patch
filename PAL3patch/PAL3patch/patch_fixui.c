#include "common.h"

// ui configuration (automatic transform)
static fRECT game_frect_ui_auto;
static double ui_scalefactor; // parameter

// ui configuration (manual transform)
static fRECT game_frect_ui_manual;
static int ui_borderflag; // parameter



// fixui state
struct fixui_state {
    fRECT src_frect, dst_frect;
    int lr_method, tb_method;
    double len_factor;
    struct fixui_state *prev;
};
static struct fixui_state def_fs; // default fixui state, at stack bottom, also for default cursor rect
static struct fixui_state *fs = &def_fs; // fixui state, stack top

// set default fixui state
static void fixui_setdefaultstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    def_fs.src_frect = *src_frect;
    def_fs.dst_frect = *dst_frect;
    def_fs.lr_method = lr_method;
    def_fs.tb_method = tb_method;
    def_fs.len_factor = len_factor;
    def_fs.prev = NULL;
}
// push a new state to stack, use this state for current state
static void fixui_pushstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    struct fixui_state *cur = malloc(sizeof(struct fixui_state));
    cur->src_frect = *src_frect;
    cur->dst_frect = *dst_frect;
    cur->lr_method = lr_method;
    cur->tb_method = tb_method;
    cur->len_factor = len_factor;
    cur->prev = fs;
    fs = cur;
}
// pop current state from stack
static void fixui_popstate()
{
    if (fs == &def_fs) { plog("can't pop default state from stack."); return; }
    // remove stack top item
    struct fixui_state *cur = fs;
    fs = cur->prev;
    free(cur);
}
// adjust structures
static void fixui_adjust_fRECT(fRECT *out_frect, fRECT *frect)
{
    transform_frect(out_frect, frect, &fs->src_frect, &fs->dst_frect, fs->lr_method, fs->tb_method, fs->len_factor);
}
static void fixui_adjust_gbUIQuad(struct gbUIQuad *out_uiquad, const struct gbUIQuad *uiquad)
{
    fRECT frect;
    set_frect_ltrb(&frect, uiquad->sx, uiquad->ey, uiquad->ex, uiquad->sy);
    fixui_adjust_fRECT(&frect, &frect);
    memcpy(out_uiquad, uiquad, sizeof(struct gbUIQuad));
    out_uiquad->sx = frect.left;
    out_uiquad->sy = frect.bottom;
    out_uiquad->ex = frect.right;
    out_uiquad->ey = frect.top;
}
/*static void fixui_adjust_RECT(RECT *out_rect, const RECT *rect)
{
    fRECT frect;
    set_frect_rect(&frect, rect);
    fixui_adjust_fRECT(&frect, &frect);
    set_rect_frect(out_rect, &frect);
}*/



// fixui default transform method functions
// default transform method will be applied when there is no other state in stack
enum fixui_default_transform {
    FIXUI_MANUAL_TRANSFORM,
    FIXUI_AUTO_TRANSFORM,
};
static int cur_def = -1;
static void fixui_setdefaultransform(int new_def)
{
    if (cur_def != new_def) {
        cur_def = new_def;
        switch (new_def) {
            case FIXUI_MANUAL_TRANSFORM:
                fixui_setdefaultstate(&game_frect, &game_frect, TR_LOW, TR_LOW, 1.0); 
                break;
            case FIXUI_AUTO_TRANSFORM:
                fixui_setdefaultstate(&game_frect_original, &game_frect_ui_auto, TR_SCALE, TR_SCALE, ui_scalefactor);
                break;
            default: fail("invalid default transform method: %d", new_def);
        }
    }
}


// PAL3 game state functions
static int fixui_map_gamestate(int pal3_gamestate)
{
    switch (pal3_gamestate) {
        case GAME_UI:
            return FIXUI_AUTO_TRANSFORM;
        default:
            return FIXUI_MANUAL_TRANSFORM;
    }
}

// check game state
static void fixui_check_gamestate()
{
    fixui_setdefaultransform(fixui_map_gamestate(game_state));
}



// hook gbDynVertBuf::RenderUIQuad()
static void *gbDynVertBuf_RenderUIQuad_original;
#define gbDynVertBuf_RenderUIQuad(this, uiquad, count, render_effect, tex_array) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gbDynVertBuf_RenderUIQuad_original, void, struct gbDynVertBuf *, struct gbUIQuad *, int, struct gbRenderEffect *, struct gbTextureArray *), this, uiquad, count, render_effect, tex_array)
static void __fastcall gbDynVertBuf_RenderUIQuad_wrapper(struct gbDynVertBuf *this, int dummy, struct gbUIQuad *uiquad, int count, struct gbRenderEffect *render_effect, struct gbTextureArray *tex_array)
{
    fixui_check_gamestate();
    struct gbUIQuad tmp_uiquad;
    fixui_adjust_gbUIQuad(&tmp_uiquad, uiquad);
    gbDynVertBuf_RenderUIQuad(this, &tmp_uiquad, count, render_effect, tex_array);
}
static void hook_gbDynVertBuf_RenderUIQuad()
{
    unsigned func_iat_entry = 0x0056A36C;
    void *warpper_ptr = gbDynVertBuf_RenderUIQuad_wrapper;
    
    // save original function pointer to gbDynVertBuf_RenderUIQuad_original
    memcpy_from_process(&gbDynVertBuf_RenderUIQuad_original, func_iat_entry, sizeof(void *));
    
    // copy wrapper function pointer to IAT entry
    memcpy_to_process(func_iat_entry, &warpper_ptr, sizeof(void *));
}












// hook GetCursorPos for cursor virtualization
// src_frect should be original rect in game
// dst_frect should be rect on screen
static fRECT src_cursor_frect, dst_cursor_frect;
static int cursor_virt_flag = 0;

static void set_cursor_virtualization(fRECT *src_frect, fRECT *dst_frect)
{
    if (src_frect && dst_frect) {
        cursor_virt_flag = 1;
        src_cursor_frect = *src_frect;
        dst_cursor_frect = *dst_frect;
    } else {
        cursor_virt_flag = 0;
    }
}

static void getcursorpos_virtualization_hookfunc()
{
    if (!getcursorpos_hook_ret) return;
    fixui_check_gamestate();
    fRECT *src_frect, *dst_frect;
    if (cursor_virt_flag) {
        src_frect = &src_cursor_frect;
        dst_frect = &dst_cursor_frect;
    } else { // use rect in def_fs for default
        src_frect = &def_fs.src_frect;
        dst_frect = &def_fs.dst_frect;
    }
    getcursorpos_hook_lppoint->x = round((getcursorpos_hook_lppoint->x - dst_frect->left) / get_frect_width(dst_frect) * get_frect_width(src_frect) + src_frect->left);
    getcursorpos_hook_lppoint->y = round((getcursorpos_hook_lppoint->y - dst_frect->top) / get_frect_height(dst_frect) * get_frect_height(src_frect) + src_frect->top);
}












// UIWnd position-tag patch,  ptag = position tag
#define UIWND_PTAG_MAGIC "PT"
#define UIWND_PTAG_MAGIC_LEN 2
#define get_ptag_with_magic(this) ((unsigned)((this)->m_bcreateok))
#define get_ptag(this) (*(struct uiwnd_ptag *)(((char *) &(this)->m_bcreateok) + UIWND_PTAG_MAGIC_LEN))
static int verify_ptag_magic(struct UIWnd *this)
{
    if (memcmp(&(this)->m_bcreateok, UIWND_PTAG_MAGIC, UIWND_PTAG_MAGIC_LEN) == 0) {
        return 1;
    } else {
        plog("position tag %08X for UIWnd %p is broken.", get_ptag_with_magic(this), this);
        return 0;
    }
}

static void push_ptag_state(struct UIWnd *father, struct UIWnd *child, struct uiwnd_ptag ptag)
{
    if (!ptag.enabled) return;

    fRECT src_frect, dst_frect;
    double len_factor;
    set_frect_rect(&src_frect, &child->m_rect);
    len_factor = scalefactor_table[ptag.scalefactor_index];
    
    fRECT *trans_src_frect, *trans_dst_frect;
    switch (ptag.self_srcrect_type) {
        case PTR_GAMERECT:           trans_src_frect = &game_frect; break;
        case PTR_GAMERECT_43:        trans_src_frect = &game_frect_43; break;
        case PTR_GAMERECT_ORIGINAL:  trans_src_frect = &game_frect_original; break;
        case PTR_INHERIT: default:   trans_src_frect = &fs->src_frect; break;
    }
    switch (ptag.self_dstrect_type) {
        case PTR_GAMERECT:           trans_dst_frect = &game_frect; break;
        case PTR_GAMERECT_43:        trans_dst_frect = &game_frect_43; break;
        case PTR_GAMERECT_ORIGINAL:  trans_dst_frect = &game_frect_original; break;
        case PTR_INHERIT: default:   trans_dst_frect = &fs->dst_frect; break;
    }
    
    set_frect_rect(&dst_frect, &child->m_rect);
    transform_frect(&dst_frect, &dst_frect, trans_src_frect, trans_dst_frect, ptag.self_lr_method, ptag.self_tb_method, len_factor);
    
    fixui_pushstate(&src_frect, &dst_frect, TR_SCALE, TR_SCALE, len_factor);
}
static void pop_ptag_state(struct UIWnd *father, struct UIWnd *child, struct uiwnd_ptag ptag)
{
    if (!ptag.enabled) return;
    fixui_popstate();
}

void set_uiwnd_ptag(struct UIWnd *this, struct uiwnd_ptag ptag)
{
    if (verify_ptag_magic(this)) {
        get_ptag(this) = ptag;
    }
}
static void __fastcall UIWnd_Render(struct UIWnd *this, int dummy)
{
    if (!this->m_bvisible) return;
    int i;
    for (i = 0; i < this->m_childs.m_nSize; i++) {
        struct UIWnd *pwnd = this->m_childs.m_pData[i];
        if (!pwnd->m_bvisible) continue;
        if (!verify_ptag_magic(pwnd)) {
            // the magic is broken due to unknown reasons
            // for safety, render only
            UIWnd_vfptr_Render(pwnd);
        } else {
            // push state, render, pop state
            push_ptag_state(this, pwnd, get_ptag(pwnd));
            UIWnd_vfptr_Render(pwnd);
            pop_ptag_state(this, pwnd, get_ptag(pwnd));
        }
    }
}

static void init_uiwnd_positiontag_patch()
{
    // asserts
    if (sizeof(struct uiwnd_ptag) != 2) fail("struct uiwnd_ptag is too big!");
    
    // modify UIWnd::Create
    SIMPLE_PATCH(0x00445BDA, "\x89\x46\x34", "\xEB\x0B\x90", 3);
    SIMPLE_PATCH(0x00445BE7, "\x90\x90\x90\x90\x90\x90\x90\x90\x90", "\xC7\x46\x34" UIWND_PTAG_MAGIC "\x00\x00\xEB\xED", 9);

    // replace UIWnd::Render
    make_jmp(0x00445CD0, UIWnd_Render);
}







// replacefont
static int uireplacefontflag;
enum {
    D3DXFONT_U12, // UNICODE 12
    D3DXFONT_U16,
    D3DXFONT_U20,
    D3DXFONT_U12S, // UNICODE 12 scaled
    D3DXFONT_U16S,
    D3DXFONT_U20S,
    D3DXFONT_COUNT // EOF
};
static DWORD d3dxfont_charset;
static DWORD d3dxfont_quality;
static LPWSTR d3dxfont_facename;
static int d3dxfont_sizelist_orig[D3DXFONT_COUNT] = {12, 16, 20, 12, 16, 20};
static int d3dxfont_sizelist[D3DXFONT_COUNT];
static int d3dxfont_boldflag[D3DXFONT_COUNT];
static ID3DXFont *d3dxfont_fontlist[D3DXFONT_COUNT] = {};
static ID3DXSprite *d3dxfont_sprite = NULL;
static IDirect3DStateBlock9 *d3dxfont_stateblock = NULL;

static int d3dxfont_selectbysize(int fontsize)
{
    int scale_flag;
    switch (cur_def) {
        case FIXUI_MANUAL_TRANSFORM: scale_flag = 0; break;
        case FIXUI_AUTO_TRANSFORM: scale_flag = 1; break;
        default: fail("invalid default transform method: %d", cur_def);
    }
    
    if (fontsize == d3dxfont_sizelist_orig[D3DXFONT_U12]) {
        return scale_flag ? D3DXFONT_U12S : D3DXFONT_U12;
    }
    if (fontsize == d3dxfont_sizelist_orig[D3DXFONT_U16]) {
        return scale_flag ? D3DXFONT_U16S : D3DXFONT_U16;
    }
    if (fontsize == d3dxfont_sizelist_orig[D3DXFONT_U20]) {
        return scale_flag ? D3DXFONT_U20S : D3DXFONT_U20;
    }
    
    return scale_flag ? D3DXFONT_U16S : D3DXFONT_U16;
}
static void d3dxfont_init()
{
    // the init function must called after IDirect3DDevice is initialized
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        int fontsize = d3dxfont_sizelist[i];
        int boldflag = d3dxfont_boldflag[i];
        if (FAILED(D3DXCreateFontW(g_GfxMgr->m_pd3dDevice, fontsize, 0, (boldflag ? FW_BOLD : 0), 0, FALSE, d3dxfont_charset, OUT_DEFAULT_PRECIS, d3dxfont_quality, DEFAULT_PITCH | FF_DONTCARE, d3dxfont_facename, &d3dxfont_fontlist[i]))) {
            fail("can't create ID3DXFont for size '%d'.", fontsize);
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
    SetRect(&rc, left, top, 0, 0);
    ID3DXFont_DrawTextW(d3dxfont_fontlist[fontid], sprite, wstr, -1, &rc, DT_NOCLIP, color);
}

static void d3dxfont_onlostdevice()
{
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        ID3DXFont_OnLostDevice(d3dxfont_fontlist[i]);
    }
    ID3DXSprite_OnLostDevice(d3dxfont_sprite);
    IDirect3DStateBlock9_Release(d3dxfont_stateblock);
}
static void d3dxfont_onresetdevice()
{
    int i;
    for (i = 0; i < D3DXFONT_COUNT; i++) {
        ID3DXFont_OnResetDevice(d3dxfont_fontlist[i]);
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
    fixui_check_gamestate();
    
    // make a node
    struct d3dxfont_strnode *node = malloc(sizeof(struct d3dxfont_strnode));
    node->fontid = d3dxfont_selectbysize(this->fontsize);
    node->wstr = cs2wcs(str, target_codepage);
    node->color = this->curColor.Color;
        
    // calc coord
    fRECT frect;
    set_frect_ltwh(&frect, (x + 1.0) * gfxdrvinfo.width / 2.0, (1.0 - y) * gfxdrvinfo.height / 2.0 - d3dxfont_sizelist_orig[node->fontid], 0, 0);
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

static void ui_replacefont_init()
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
    
    // init sizes
    d3dxfont_sizelist[D3DXFONT_U12S] = floor(d3dxfont_sizelist[D3DXFONT_U12] * ui_scalefactor);
    d3dxfont_sizelist[D3DXFONT_U16S] = floor(d3dxfont_sizelist[D3DXFONT_U16] * ui_scalefactor);
    d3dxfont_sizelist[D3DXFONT_U20S] = floor(d3dxfont_sizelist[D3DXFONT_U20] * ui_scalefactor);
    
    // init boldflags
    d3dxfont_boldflag[D3DXFONT_U12S] = d3dxfont_boldflag[D3DXFONT_U12];
    d3dxfont_boldflag[D3DXFONT_U16S] = d3dxfont_boldflag[D3DXFONT_U16];
    d3dxfont_boldflag[D3DXFONT_U20S] = d3dxfont_boldflag[D3DXFONT_U20];
    
    // add hooks
    add_postd3dcreate_hook(d3dxfont_init);
    add_onlostdevice_hook(d3dxfont_onlostdevice);
    add_onresetdevice_hook(d3dxfont_onresetdevice);
    
    // hook gbPrintFont_UNICODE's member function
    make_jmp(gboffset + 0x10023A90, gbPrintFont_UNICODE_Flush);
    make_jmp(gboffset + 0x10023FD0, gbPrintFont_UNICODE_PrintString);
}























void haha()//////////////////////////////////////////////////////FIXME
{
    printf("%p %p\n", fixui_pushstate, fixui_popstate);
}


MAKE_PATCHSET(fixui)
{
    // calc game_frect_ui_auto
    game_frect_ui_auto = game_frect_original;
    const char *usf = get_string_from_configfile("uiscalefactor");
    ui_scalefactor = stricmp(usf, "max") == 0 ? game_scalefactor : fmin(game_scalefactor, str2double(usf));
    transform_frect(&game_frect_ui_auto, &game_frect_ui_auto, &game_frect, &game_frect, TR_CENTER, TR_CENTER, ui_scalefactor);
    
    // calc game_frect_ui_manual
    ui_borderflag = get_int_from_configfile("uiborder");
    game_frect_ui_manual = ui_borderflag ? game_frect_43 : game_frect;
    
    // read font scale configuration
    uireplacefontflag = get_int_from_configfile("uireplacefont");
    if (uireplacefontflag) {
        // this function must called when uiscalefactor is set
        ui_replacefont_init();
    }
    
    // init fixui, use identity transform
    fixui_setdefaultransform(FIXUI_MANUAL_TRANSFORM);
    hook_gbDynVertBuf_RenderUIQuad();
    
    // init cursor virtualization
    set_cursor_virtualization(NULL, NULL);
    add_getcursorpos_hook(getcursorpos_virtualization_hookfunc);
    
    // init uiwnd position tag patch
    init_uiwnd_positiontag_patch();
}
