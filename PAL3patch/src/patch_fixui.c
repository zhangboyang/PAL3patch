#include "common.h"

// ui configuration (automatic transform)
static fRECT game_frect_ui_auto;

// fixui state
static struct fixui_state def_fs; // default fixui state, at stack bottom, also for default cursor rect
struct fixui_state *fs = &def_fs; // fixui state, stack top

// set default fixui state
static void fixui_setdefaultstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    def_fs.src_frect = *src_frect;
    def_fs.dst_frect = *dst_frect;
    def_fs.lr_method = lr_method;
    def_fs.tb_method = tb_method;
    def_fs.len_factor = len_factor;
    def_fs.no_cursor_virt = 0;
    def_fs.prev = NULL;
}
// allocate and construct a new state struct
struct fixui_state *fixui_newstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    struct fixui_state *cur = malloc(sizeof(struct fixui_state));
    cur->src_frect = *src_frect;
    cur->dst_frect = *dst_frect;
    cur->lr_method = lr_method;
    cur->tb_method = tb_method;
    cur->len_factor = len_factor;
    cur->no_cursor_virt = 0;
    cur->no_align = 0;
    cur->prev = NULL;
    return cur;
}
// duplicate current state
struct fixui_state *fixui_dupstate()
{
    struct fixui_state *ptr = malloc(sizeof(struct fixui_state));
    *ptr = *fs;
    ptr->prev = NULL;
    return ptr;
}
// push a new state to stack, use this state for current state
void fixui_pushstate_node(struct fixui_state *cur)
{
    if (cur->prev != NULL) fail("double push detected.");
    cur->prev = fs;
    fs = cur;
}
// pop current state from stack
void fixui_popstate()
{
    if (fs == &def_fs) { plog("can't pop default state from stack."); return; }
    // remove stack top item
    struct fixui_state *cur = fs;
    fs = cur->prev;
    free(cur);
}
// adjust structures
void fixui_adjust_fRECT(fRECT *out_frect, const fRECT *frect)
{
    transform_frect(out_frect, frect, &fs->src_frect, &fs->dst_frect, fs->lr_method, fs->tb_method, fs->len_factor);
}
void fixui_adjust_gbUIQuad(struct gbUIQuad *out_uiquad, const struct gbUIQuad *uiquad)
{
    fRECT frect;
    set_frect_ltrb(&frect, uiquad->sx, uiquad->ey, uiquad->ex, uiquad->sy);
    fixui_adjust_fRECT(&frect, &frect);
    
    double left = frect.left;
    double top = frect.top;
    if (!fs->no_align) {
        if (fcmp(fs->len_factor, 1.0) == 0) {
            left = floor(left + eps);
            top = floor(top + eps);
        }
        left -= 0.5;
        top -= 0.5;
    }
    double width = frect.right - frect.left;
    double height = frect.bottom - frect.top;

    memcpy(out_uiquad, uiquad, sizeof(struct gbUIQuad));
    out_uiquad->sx = left;
    out_uiquad->sy = top + height;
    out_uiquad->ex = left + width;
    out_uiquad->ey = top;
}
void fixui_adjust_RECT(RECT *out_rect, const RECT *rect)
{
    fRECT frect;
    set_frect_rect(&frect, rect);
    fixui_adjust_fRECT(&frect, &frect);
    set_rect_frect(out_rect, &frect);
}
void fixui_adjust_fPOINT(fPOINT *out_fpoint, const fPOINT *fpoint)
{
    fRECT frect;
    set_frect_ltwh(&frect, fpoint->x, fpoint->y, 0.0, 0.0);
    fixui_adjust_fRECT(&frect, &frect);
    set_fpoint(out_fpoint, frect.left, frect.top);
}
void fixui_adjust_POINT(POINT *out_point, const POINT *point)
{
    fPOINT fpoint;
    set_fpoint_point(&fpoint, point);
    fixui_adjust_fPOINT(&fpoint, &fpoint);
    set_point_fpoint(out_point, &fpoint);
}


// point scale functions
void fixui_scale_fPOINT(fPOINT *out_fpoint, fPOINT *fpoint, fRECT *src_frect, fRECT *dst_frect)
{
    // note: out_fpoint might equals to fpoint
    out_fpoint->x = (fpoint->x - dst_frect->left) / get_frect_width(dst_frect) * get_frect_width(src_frect) + src_frect->left;
    out_fpoint->y = (fpoint->y - dst_frect->top) / get_frect_height(dst_frect) * get_frect_height(src_frect) + src_frect->top;
}
void fixui_scale_POINT_round(POINT *out_point, POINT *point, fRECT *src_frect, fRECT *dst_frect)
{
    fPOINT fpoint;
    set_fpoint_point(&fpoint, point);
    fixui_scale_fPOINT(&fpoint, &fpoint, src_frect, dst_frect);
    set_point_fpoint_round(out_point, &fpoint);
}


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
                fixui_setdefaultstate(&game_frect_original, &game_frect_ui_auto, TR_SCALE_LOW, TR_SCALE_LOW, ui_scalefactor);
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

// check and update game state
void fixui_update_gamestate()
{
    fixui_setdefaultransform(fixui_map_gamestate(PAL3_s_gamestate));
}



// hook gbDynVertBuf::RenderUIQuad()
static void *gbDynVertBuf_RenderUIQuad_original;
#define gbDynVertBuf_RenderUIQuad(this, uiquad, count, render_effect, tex_array) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gbDynVertBuf_RenderUIQuad_original, void, struct gbDynVertBuf *, struct gbUIQuad *, int, struct gbRenderEffect *, struct gbTextureArray *), this, uiquad, count, render_effect, tex_array)
static MAKE_THISCALL(void, gbDynVertBuf_RenderUIQuad_wrapper, struct gbDynVertBuf *this, struct gbUIQuad *uiquad, int count, struct gbRenderEffect *render_effect, struct gbTextureArray *tex_array)
{
    fixui_update_gamestate();
    struct gbUIQuad *tmp_uiquad = malloc(sizeof(struct gbUIQuad) * count);
    int i;
    for (i = 0; i < count; i++) {
        fixui_adjust_gbUIQuad(&tmp_uiquad[i], &uiquad[i]);
    }
    gbDynVertBuf_RenderUIQuad(this, tmp_uiquad, count, render_effect, tex_array);
    free(tmp_uiquad);
}
static void hook_gbDynVertBuf_RenderUIQuad()
{
    unsigned func_iat_entry = 0x0056A36C;
    
    // save original function pointer to gbDynVertBuf_RenderUIQuad_original
    memcpy_from_process(&gbDynVertBuf_RenderUIQuad_original, func_iat_entry, sizeof(void *));
    
    // copy wrapper function pointer to IAT entry
    INIT_WRAPPER_VFPTR(gbDynVertBuf_RenderUIQuad_wrapper, func_iat_entry);
    
    // fix calls inside GBENGINE.DLL
    INIT_WRAPPER_CALL(gbDynVertBuf_RenderUIQuad_wrapper, {
        gboffset + 0x10015476,
        gboffset + 0x100229A7,
        // gboffset + 0x10023462, // NOTE: should NOT hook this function in gbPrintFont_NUM::Flush(), see notes20161115.txt for details
        gboffset + 0x10023A87,
    });
}






// gbPrintFont::PrintString wrapper for derived classes (not for gbPrintFont_UNICODE)
static MAKE_THISCALL(void, gbPrintFont_PrintString_wrapper, struct gbPrintFont *this, const char *str, float x, float y, float endx, float endy)
{
    fPOINT a = {gbx2x(x), gby2y(y)}, b = {gbx2x(endx), gby2y(endy)};
    fixui_adjust_fPOINT(&a, &a);
    fixui_adjust_fPOINT(&b, &b);
    
    float ScaleX_sv = this->ScaleX, ScaleY_sv = this->ScaleY;
    this->ScaleX *= fs->len_factor;
    this->ScaleY *= fs->len_factor;
    gbPrintFont_PrintString(this, str, x2gbx(a.x), y2gby(a.y), x2gbx(b.x), y2gby(b.y));
    this->ScaleX = ScaleX_sv;
    this->ScaleY = ScaleY_sv;
}

static void hook_gbPrintFont_PrintString()
{
    // we only patch gbPrintFont_NUM here
    // FIXME: should we patch more classes? (e.g. gbPrintFont_ASC)
    INIT_WRAPPER_VFPTR(gbPrintFont_PrintString_wrapper, gboffset + 0x100F586C); // vfptr for gbPrintFont_NUM
}





// hook GetCursorPos for cursor virtualization
static void set_cursor_virt(int enabled)
{
    fs->no_cursor_virt = !enabled;
}
static void getcursorpos_virtualization_hookfunc(void *arg)
{
    POINT *mousept = arg;
    fixui_update_gamestate();
    if (!fs->no_cursor_virt) {
        // src_frect should be original rect in game
        // dst_frect should be rect on screen
        fixui_scale_POINT_round(mousept, mousept, &fs->src_frect, &fs->dst_frect);
    }
}



// fill auto transform ui border
static int fillborder_enabled;
static IDirect3DStateBlock9 *uifb_stateblock;
static IDirect3DVertexBuffer9 *uifb_vbuf = NULL;
struct uifb_vertex_t {
    float x, y, z, rhw;
    DWORD color;
};
#define UIFB_VERTEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define UIFB_VERTEX_SIZE (sizeof(struct uifb_vertex_t))
#define UIFB_VBUF_TRANGLE_PER_RECT 2
#define UIFB_VBUF_RECT_CNT 4
#define UIFB_VBUF_SIZE (UIFB_VBUF_TRANGLE_PER_RECT * UIFB_VBUF_RECT_CNT * 3)
#define UIFB_VBUF_SIZE_BYTES (UIFB_VERTEX_SIZE * UIFB_VBUF_SIZE)
static void vbuf_put_frect(struct uifb_vertex_t *vbuf, const fRECT *frect)
{
    float left = frect->left;
    float top = frect->top;
    float right = frect->right;
    float bottom = frect->bottom;
    left = floor(left + eps) - 0.5f;
    right = ceil(right - eps) - 0.5f;
    top = floor(top + eps) - 0.5f;
    bottom = ceil(bottom - eps) - 0.5f;
    vbuf[0].x = vbuf[5].x = left;
    vbuf[0].y = vbuf[5].y = top;
    vbuf[1].x = left;
    vbuf[1].y = bottom;
    vbuf[2].x = vbuf[3].x = right;
    vbuf[2].y = vbuf[3].y = bottom;
    vbuf[4].x = right;
    vbuf[4].y = top;
    vbuf[0].color = vbuf[1].color = vbuf[2].color = vbuf[3].color = vbuf[4].color = vbuf[5].color = 0;
    vbuf[0].rhw = vbuf[1].rhw = vbuf[2].rhw = vbuf[3].rhw = vbuf[4].rhw = vbuf[5].rhw = 1.0f;
    vbuf[0].z = vbuf[1].z = vbuf[2].z = vbuf[3].z = vbuf[4].z = vbuf[5].z = 0.0f;
}
static void vbuf_draw(const fRECT *frect_list, int frect_cnt)
{
    IDirect3DStateBlock9_Capture(uifb_stateblock);
    
    // upload vertex
    int i;
    struct uifb_vertex_t *vbuf;
    IDirect3DVertexBuffer9_Lock(uifb_vbuf, 0, 0, (void *) &vbuf, 0);
    int draw_cnt = 0;
    for (i = 0; i < frect_cnt; i++) {
        if (fcmp(frect_list[i].left, frect_list[i].right) != 0 && fcmp(frect_list[i].top, frect_list[i].bottom) != 0) {
            vbuf_put_frect(vbuf + UIFB_VBUF_TRANGLE_PER_RECT * 3 * draw_cnt, &frect_list[i]);
            draw_cnt++;
        }
    }
    IDirect3DVertexBuffer9_Unlock(uifb_vbuf);
    
    if (draw_cnt > 0) {
        // prepare d3d state
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ALPHABLENDENABLE, FALSE);
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_CULLMODE, D3DCULL_NONE);
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ZENABLE, TRUE);
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_ZFUNC, D3DCMP_ALWAYS);
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_LIGHTING, FALSE);
        IDirect3DDevice9_SetRenderState(GB_GfxMgr->m_pd3dDevice, D3DRS_SHADEMODE, D3DSHADE_FLAT);
        IDirect3DDevice9_SetTextureStageState(GB_GfxMgr->m_pd3dDevice, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        
        // draw
        IDirect3DDevice9_SetFVF(GB_GfxMgr->m_pd3dDevice, UIFB_VERTEX_FVF);
        IDirect3DDevice9_SetStreamSource(GB_GfxMgr->m_pd3dDevice, 0, uifb_vbuf, 0, UIFB_VERTEX_SIZE);
        IDirect3DDevice9_DrawPrimitive(GB_GfxMgr->m_pd3dDevice, D3DPT_TRIANGLELIST, 0, UIFB_VBUF_TRANGLE_PER_RECT * draw_cnt);
    }

    IDirect3DStateBlock9_Apply(uifb_stateblock);
}
static void uifb_draw()
{
    double lr[] = { game_frect.left, game_frect_ui_auto.left, game_frect_ui_auto.right, game_frect.right };
    double tb[] = { game_frect.top, game_frect_ui_auto.top, game_frect_ui_auto.bottom, game_frect.bottom };
    fRECT frect[4];
    set_frect_ltrb(&frect[0], lr[0], tb[0], lr[1], tb[3]);
    set_frect_ltrb(&frect[1], lr[2], tb[0], lr[3], tb[3]);
    set_frect_ltrb(&frect[2], lr[0], tb[0], lr[3], tb[1]);
    set_frect_ltrb(&frect[3], lr[0], tb[2], lr[3], tb[3]);
    vbuf_draw(frect, 4);
}
static void uifb_create()
{
    if (FAILED(IDirect3DDevice9_CreateVertexBuffer(GB_GfxMgr->m_pd3dDevice, UIFB_VBUF_SIZE_BYTES, 0, UIFB_VERTEX_FVF, D3DPOOL_MANAGED, &uifb_vbuf, NULL))) {
        fail("can't create vertex buffer for filling ui border.");
    }
    if (FAILED(IDirect3DDevice9_CreateStateBlock(GB_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &uifb_stateblock))) {
        fail("can't create state block for filling ui border.");
    }
}
static void uifb_onresetdevice()
{
    if (FAILED(IDirect3DDevice9_CreateStateBlock(GB_GfxMgr->m_pd3dDevice, D3DSBT_ALL, &uifb_stateblock))) {
        fail("can't create state block for filling ui border.");
    }
}
static void uifb_onlostdevice()
{
    IDirect3DStateBlock9_Release(uifb_stateblock);
}
static void uifb_precursordraw()
{
    // this function may be called when fillborder is disabled
    if (fillborder_enabled && cur_def == FIXUI_AUTO_TRANSFORM) {
        uifb_draw();
    }
}
static void init_fillborder()
{
    fillborder_enabled = get_int_from_configfile("uifillborder");
    if (!fillborder_enabled) return;
    add_postd3dcreate_hook(uifb_create);
    add_onlostdevice_hook(uifb_onlostdevice);
    add_onresetdevice_hook(uifb_onresetdevice);
}







// hook UICursor_IRender to change cursor size (only in soft-cursor mode)
static MAKE_THISCALL(void, UICursor_IRender_wrapper, struct UICursor *this)
{
    uifb_precursordraw();
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, softcursor_scalefactor);
    UICursor_IRender(this);
    fixui_popstate();
}
static void init_softcursor_sizepatch()
{
    softcursor_scalefactor = str2scalefactor(get_string_from_configfile("softcursor_scalefactor"));
    INIT_WRAPPER_CALL(UICursor_IRender_wrapper, { 0x004061D2 });
}








// UIWnd position-tag patch,  ptag = position tag
#define get_ptag_raw(this) ((unsigned)((this)->m_bcreateok))
#define get_ptag(this) (*(struct uiwnd_ptag *)(&(this)->m_bcreateok))

static int verify_ptag_magic(struct UIWnd *this)
{
    if (get_ptag(this).magic == UIWND_PTAG_MAGIC) {
        return 1;
    } else {
        warning("position tag %08X for UIWnd %p is broken.", get_ptag_raw(this), this);
        return 0;
    }
}

void push_ptag_state(struct UIWnd *pwnd)
{
    if (!verify_ptag_magic(pwnd)) return;
    struct uiwnd_ptag ptag = get_ptag(pwnd);
    if (!ptag.enabled) return;
    
    // check in-use flag
    if (ptag.in_use) {
        warning("duplicate ptag in stack.");
    }
    
    // read rect information from ptag
    fRECT *trans_src_frect, *trans_dst_frect;
    trans_src_frect = get_ptag_frect(ptag.self_srcrect_type);
    if (!trans_src_frect) fail("invalid ptag srcrect type %d.", ptag.self_srcrect_type);
    trans_dst_frect = get_ptag_frect(ptag.self_dstrect_type);
    if (!trans_dst_frect) fail("invalid ptag dstrect type %d.", ptag.self_dstrect_type);
    
    // transform window rect using ptag
    fRECT src_frect, dst_frect;
    double len_factor;
    set_frect_rect(&src_frect, &pwnd->m_rect);
    len_factor = scalefactor_table[ptag.scalefactor_index];
    set_frect_rect(&dst_frect, &pwnd->m_rect);
    transform_frect(&dst_frect, &dst_frect, trans_src_frect, trans_dst_frect, ptag.self_lr_method, ptag.self_tb_method, len_factor);
    
    // scale window contents
    fixui_pushstate(&src_frect, &dst_frect, TR_SCALE_LOW, TR_SCALE_LOW, len_factor);
    if (ptag.no_cursor_virt) set_cursor_virt(0); // update cursor virt status
    
    // update in-use flag
    get_ptag(pwnd).in_use = 1;
}
void pop_ptag_state(struct UIWnd *pwnd)
{
    if (!verify_ptag_magic(pwnd)) return;
    struct uiwnd_ptag ptag = get_ptag(pwnd);
    if (!ptag.enabled) return;
    
    // restore in-use flag
    if (!ptag.in_use) {
        warning("unmatched ptag pop operation.");
    }
    get_ptag(pwnd).in_use = 0;
    
    // pop state
    fixui_popstate();
}

void set_uiwnd_ptag(struct UIWnd *this, struct uiwnd_ptag ptag)
{
    if (verify_ptag_magic(this)) {
        get_ptag(this) = ptag;
    }
}
static MAKE_THISCALL(void, UIWnd_Render, struct UIWnd *this)
{
    if (!this->m_bvisible) return;
    int i;
    for (i = 0; i < this->m_childs.m_nSize; i++) {
        struct UIWnd *pwnd = this->m_childs.m_pData[i];
        if (!pwnd->m_bvisible) continue;
        if (!verify_ptag_magic(pwnd) || !verify_ptag_magic(this)) {
            // the magic is broken due to unknown reasons
            // for safety, render only
            UIWnd_vfptr_Render(pwnd);
        } else {
            // push state, render, pop state
            if (get_ptag(this).self_only_ptag) pop_ptag_state(this);
            push_ptag_state(pwnd);
            UIWnd_vfptr_Render(pwnd);
            pop_ptag_state(pwnd);
            if (get_ptag(this).self_only_ptag) push_ptag_state(this);
        }
    }
}
static MAKE_THISCALL(int, UIWnd_Update, struct UIWnd *this, float deltatime, int haveinput)
{
    if (!this->m_bvisible || !this->m_benable) return 0;
    int i;
    for (i = this->m_childs.m_nSize - 1; i >= 0; i--) {
        struct UIWnd *pwnd = this->m_childs.m_pData[i];
        int ret;
        if (!verify_ptag_magic(pwnd) || !verify_ptag_magic(this)) {
            // fallback    
            ret = UIWnd_vfptr_Update(pwnd, deltatime, haveinput);
        } else {
            // push state, update, pop state
            if (get_ptag(this).self_only_ptag) pop_ptag_state(this);
            push_ptag_state(pwnd);
            ret = UIWnd_vfptr_Update(pwnd, deltatime, haveinput);
            pop_ptag_state(pwnd);
            if (get_ptag(this).self_only_ptag) push_ptag_state(this);
        }
        if (ret) haveinput = 0;
    }
    return !haveinput;
}

static void init_uiwnd_positiontag_patch()
{
    // modify UIWnd::Create
    SIMPLE_PATCH(0x00445BDA, "\x89\x46\x34", "\xEB\x0B\x90", 3);
    
    char code_with_magic[] = "\xC7\x46\x34\x00\x00\x00\x00\xEB\xED";
    struct uiwnd_ptag initial_ptag;
    memset(&initial_ptag, 0, sizeof(struct uiwnd_ptag));
    initial_ptag.magic = UIWND_PTAG_MAGIC;
    memcpy(code_with_magic + 3, &initial_ptag, sizeof(struct uiwnd_ptag));
    SIMPLE_PATCH(0x00445BE7, "\x90\x90\x90\x90\x90\x90\x90\x90\x90", code_with_magic, 9);

    // replace UIWnd::Update and UIWnd::Render
    make_jmp(0x00445C60, UIWnd_Update);
    make_jmp(0x00445CD0, UIWnd_Render);
}








fRECT *get_ptag_frect(int rect_type)
{
    switch (rect_type) {
        case PTR_GAMERECT:           return &game_frect;
        case PTR_GAMERECT_43:        return &game_frect_43;
        case PTR_GAMERECT_ORIGINAL:  return &game_frect_original;
        case PTR_GAMERECT_UIAUTO:    return &game_frect_ui_auto;
        default:
            if (PTR_GAMERECT_CUSTOM0 <= rect_type && rect_type < PTR_GAMERECT_CUSTOM0 + MAX_CUSTOM_GAME_FRECT) {
                return &game_frect_custom[rect_type - PTR_GAMERECT_CUSTOM0];
            }
            return NULL;
    }
}
int parse_uiwnd_rect_type(const char *str)
{
    int customrect;
    if (stricmp(str, "full") == 0) {
        return PTR_GAMERECT;
    } else if (stricmp(str, "full43") == 0) {
        return PTR_GAMERECT_43;
    } else if (strnicmp(str, "custom", 6) == 0 && sscanf(str + 6, "%d", &customrect) == 1 && 0 <= customrect && customrect < MAX_CUSTOM_GAME_FRECT) {
        return PTR_GAMERECT_CUSTOM0 + customrect;
    } else {
        fail("unknown rect type %s.", str);
    }
}



static void init_align_uirect()
{
    // fix 0.5 and -0.5 to 0
    PATCH_FLOAT_MEMREF_EXPR(0.0f, {
        // UIRenderQuad_2Rect_byFVF
        0x005402CD,
        0x00540303,
        0x00540315,
        0x0054038E,
        0x005403C7,
        
        // _Texture_Info::_CalculateUV()
        0x0053F329,
        0x0053F349,
        0x0053F303,
        0x0053F36D,
        
        // UIStaticFlex::DrawRect()
        0x004457AC,
        0x004457D9,
        
        // UIFlexBar::DrawRect()
        0x0043DBCE,
        0x0043DB52,
    });
    SIMPLE_PATCH(0x00570A6C, "\x00\x00\x00\xBF", "\x00\x00\x00\x00", 4); // patch UIRenderQuad functions
}



static void __cdecl UIRenderQuad_color_wrapper(int left, int top, int right, int bottom, float sv, float ev, struct gbColorQuad *color, struct gbTextureArray *atex)
{
    fixui_pushidentity();
    left = floor(game_frect.left + eps);
    top = floor(game_frect.top + eps);
    right = floor(game_frect.right + eps);
    bottom = floor(game_frect.bottom + eps);
    UIRenderQuad_color(left, top, right, bottom, sv, ev, color, atex);
    fixui_popstate();
}
static void hook_UIRenderQuad_color()
{
    INIT_WRAPPER_CALL(UIRenderQuad_color_wrapper, {
        0x0045995D,
        0x0047C2BD,
        0x004A487D,
        0x0052365D,
        0x00527D2D,
        0x00529CDB,
    });
}


MAKE_PATCHSET(fixui)
{
    // calc game_frect_ui_auto
    game_frect_ui_auto = game_frect_original;
    ui_scalefactor = str2scalefactor(get_string_from_configfile("uiscalefactor"));
    transform_frect(&game_frect_ui_auto, &game_frect_ui_auto, &game_frect, &game_frect, TR_CENTER, TR_CENTER, ui_scalefactor);
    
    // init fixui, use identity transform
    fixui_setdefaultransform(FIXUI_MANUAL_TRANSFORM);
    hook_gbDynVertBuf_RenderUIQuad();
    hook_gbPrintFont_PrintString(); 
    add_postpresent_hook(fixui_update_gamestate);
    
    // init cursor virtualization
    add_getcursorpos_hook(getcursorpos_virtualization_hookfunc);
    
    // init uiwnd position tag patch
    init_uiwnd_positiontag_patch();
    
    // init soft-cursor size patch
    init_softcursor_sizepatch();
    
    // init fill border
    init_fillborder();
    
    // init align uirect
    init_align_uirect();
    
    // hook UIRenderQuad_color for drawing system ui background
    hook_UIRenderQuad_color();
}
