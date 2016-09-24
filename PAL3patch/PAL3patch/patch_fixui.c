#include "common.h"

// ui configuration
static fRECT game_frect_ui;
static double ui_scalefactor;


// fixui state

struct fixui_state {
    fRECT src_frect, dst_frect;
    int lr_method, tb_method;
    double len_factor;
    struct fixui_state *prev;
};
static struct fixui_state def_fs; // default fixui state, at stack bottom, also for default cursor rect
static struct fixui_state *fs = &def_fs; // fixui state

// set default fixui state
void fixui_setdefaultstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    def_fs.src_frect = *src_frect;
    def_fs.dst_frect = *dst_frect;
    def_fs.lr_method = lr_method;
    def_fs.tb_method = tb_method;
    def_fs.len_factor = len_factor;
    def_fs.prev = NULL;
}
// push a new state to stack, use this state for current state
void fixui_pushstate(fRECT *src_frect, fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
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
void fixui_popstate()
{
    if (fs == &def_fs) { plog("can't pop default state from stack."); return; }
    // remove stack top item
    struct fixui_state *cur = fs;
    fs = cur->prev;
    free(cur);
}
// adjust structures
void fixui_adjust_fRECT(fRECT *out_frect, fRECT *frect)
{
    transform_frect(out_frect, frect, &fs->src_frect, &fs->dst_frect, fs->lr_method, fs->tb_method, fs->len_factor);
}
void fixui_adjust_gbUIQuad(struct gbUIQuad *out_uiquad, const struct gbUIQuad *uiquad)
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
void fixui_adjust_RECT(RECT *out_rect, const RECT *rect)
{
    fRECT frect;
    set_frect_rect(&frect, rect);
    fixui_adjust_fRECT(&frect, &frect);
    set_rect_frect(out_rect, &frect);
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
                fixui_setdefaultstate(&game_frect_original, &game_frect_ui, TR_SCALE, TR_SCALE, ui_scalefactor);
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

// hook UIDrawTextEx()
#define UIDrawTextEx ((void (*)(const char *, RECT *, struct gbPrintFont *, int, int)) TOPTR(0x00541210))
static void UIDrawTextEx_wrapper(const char *str, RECT *rect, struct gbPrintFont *font, int fontsize, int middleflag)
{
    fixui_check_gamestate();
    RECT tmp_rect;
    fixui_adjust_RECT(&tmp_rect, rect);
    UIDrawTextEx(str, &tmp_rect, font, fontsize, middleflag);
}
static void hook_UIDrawTextEx()
{
    // xref of UIDrawTextEx()
    make_call(0x00450533, UIDrawTextEx_wrapper);
    make_call(0x005411F2, UIDrawTextEx_wrapper);
}

// hook UIPrint()
#define UIPrint ((void (*)(int, int, char *, struct gbColorQuad *, int)) TOPTR(0x00540FD0))
static void UIPrint_wrapper(int x, int y, char *str, struct gbColorQuad *color, int fontsize)
{
    fixui_check_gamestate();
    RECT tmp_rect;
    SetRect(&tmp_rect, x, y, 0, 0);
    fixui_adjust_RECT(&tmp_rect, &tmp_rect);
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


// hook GetCursorPos for cursor virtualization
// src_frect should be original rect in game
// dst_frect should be rect on screen
static fRECT src_cursor_frect, dst_cursor_frect;
static int cursor_virt_flag = 0;

void set_cursor_virtualization(fRECT *src_frect, fRECT *dst_frect)
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
    getcursorpos_hook_lppoint->x = (getcursorpos_hook_lppoint->x - dst_frect->left) / get_frect_width(dst_frect) * get_frect_width(src_frect) + src_frect->left;
    getcursorpos_hook_lppoint->y = (getcursorpos_hook_lppoint->y - dst_frect->top) / get_frect_height(dst_frect) * get_frect_height(src_frect) + src_frect->top;
}


MAKE_PATCHSET(fixui)
{
    // calc game_ui_frect
    game_frect_ui = game_frect_original;
    ui_scalefactor = fmin(game_scalefactor, str2double(get_string_from_configfile("uiscalefactor")));
    transform_frect(&game_frect_ui, &game_frect_ui, &game_frect, &game_frect, TR_CENTER, TR_CENTER, ui_scalefactor);
    
    // init fixui, use identity transform
    fixui_setdefaultransform(FIXUI_MANUAL_TRANSFORM);
    hook_gbDynVertBuf_RenderUIQuad();
    hook_UIDrawTextEx();
    hook_UIPrint();
    
    // init cursor virtualization
    set_cursor_virtualization(NULL, NULL);
    add_getcursorpos_hook(getcursorpos_virtualization_hookfunc);
}
