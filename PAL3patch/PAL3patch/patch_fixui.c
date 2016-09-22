#include "common.h"

// UIadjust state
static fRECT *cur_new_rect;
static int cur_lr_method, cur_tb_method;
static double cur_len_factor;
static int cur_mousemode;

static void UIadjust_fRECT(fRECT *frect)
{
    transform_frect(frect, frect, &game_frect_original, cur_new_rect, cur_lr_method, cur_tb_method, cur_len_factor);
}

void UIadjust_setstate(int screen_type, int lr_method, int tb_method, double len_factor)
{
    switch (screen_type) {
        case UI_ADJUST_FULLSCREEN: cur_new_rect = &game_frect; break;
        case UI_ADJUST_BORDERED: cur_new_rect = &game_frect_43; break;
        default: fail("unknown UIadjust screen type: %d", screen_type);
    }
    cur_lr_method = lr_method;
    cur_tb_method = tb_method;
    cur_len_factor = len_factor;
}

void UIadjust_gbUIQuad(struct gbUIQuad *out_uiquad, const struct gbUIQuad *uiquad)
{
    fRECT frect;
    set_frect_ltrb(&frect, uiquad->sx, uiquad->ey, uiquad->ex, uiquad->sy);
    UIadjust_fRECT(&frect);
    memcpy(out_uiquad, uiquad, sizeof(struct gbUIQuad));
    out_uiquad->sx = frect.left;
    out_uiquad->sy = frect.bottom;
    out_uiquad->ex = frect.right;
    out_uiquad->ey = frect.top;
}
void UIadjust_RECT(RECT *out_rect, const RECT *rect)
{
    fRECT frect;
    set_frect_rect(&frect, rect);
    UIadjust_fRECT(&frect);
    set_rect_frect(out_rect, &frect);
}


static void *gbDynVertBuf_RenderUIQuad_original;
#define gbDynVertBuf_RenderUIQuad(this, uiquad, count, render_effect, tex_array) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(gbDynVertBuf_RenderUIQuad_original, void, struct gbDynVertBuf *, struct gbUIQuad *, int, struct gbRenderEffect *, struct gbTextureArray *), this, uiquad, count, render_effect, tex_array)

static void __fastcall gbDynVertBuf_RenderUIQuad_wrapper(struct gbDynVertBuf *this, int dummy, struct gbUIQuad *uiquad, int count, struct gbRenderEffect *render_effect, struct gbTextureArray *tex_array)
{
    struct gbUIQuad tmp_uiquad;
    UIadjust_gbUIQuad(&tmp_uiquad, uiquad);
    gbDynVertBuf_RenderUIQuad(this, &tmp_uiquad, count, render_effect, tex_array);
}

static void hook_gbDynVertBuf_RenderUIQuad()
{
    unsigned func_iat_entry = 0x0056A36C;
    void *warpper_ptr = gbDynVertBuf_RenderUIQuad_wrapper;
    memcpy_from_process(&gbDynVertBuf_RenderUIQuad_original, func_iat_entry, sizeof(void *));
    memcpy_to_process(func_iat_entry, &warpper_ptr, sizeof(void *));
}

static void getcursorpos_fixui_hookfunc()
{
    if (cur_mousemode == UI_MOUSE_VIRTUALIZED && cur_new_rect) {
        LONG x, y;
        x = getcursorpos_hook_lppoint->x;
        y = getcursorpos_hook_lppoint->y;
        x = (x - cur_new_rect->left) / game_scalefactor;
        y = (y - cur_new_rect->top) / game_scalefactor;
        /*if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= GAME_WIDTH_ORG) x = GAME_WIDTH_ORG - 1;
        if (y >= GAME_HEIGHT_ORG) y = GAME_HEIGHT_ORG - 1;*/
        getcursorpos_hook_lppoint->x = x;
        getcursorpos_hook_lppoint->y = y;
    }
}

MAKE_PATCHSET(fixui)
{
    // init UIadjust
    UIadjust_setstate(UI_ADJUST_BORDERED, TR_AUTO, TR_AUTO, game_scalefactor);
    
    // hook gbDynVertBuf::RenderUIQuad()
    hook_gbDynVertBuf_RenderUIQuad();
    
    // hook cursor
    cur_mousemode = UI_MOUSE_VIRTUALIZED;
    add_getcursorpos_hook(getcursorpos_fixui_hookfunc);
}
