#include "common.h"

static void __fastcall UIStaticEX_Render_wrapper(struct UIStaticEX *this, int dummy)
{
    RECT *wndrect = &((struct UIWnd *) this)->m_rect;
    RECT old_rect = *wndrect;
    fRECT new_frect, old_frect, tmp_frect;
    double len_factor = fs->len_factor;
    
    set_frect_rect(&old_frect, &old_rect);
    fixui_adjust_fRECT(&new_frect, &old_frect);
    transform_frect(&tmp_frect, &old_frect, &new_frect, &new_frect, TR_CENTER, TR_CENTER, 1.0);
    set_rect_frect(wndrect, &tmp_frect);

    fixui_pushstate(&tmp_frect, &tmp_frect, TR_SCALE_MID, TR_SCALE_MID, len_factor);
    UIStaticEX_Render(this);
    fixui_popstate();

    *wndrect = old_rect;
}

static void __fastcall UIStaticEXA_Render_wrapper(struct UIStaticEXA *this, int dummy)
{
    // UIStaticEXA is for display TaiJi pictures only
    // TaiJi pictures are in UILib
    // there is no need to adjust m_rect and fixui state very complex like UIStaticEX
    
    RECT *wndrect = &((struct UIWnd *) this)->m_rect;
    RECT old_rect = *wndrect;
    POINT old_pt = { this->center_x, this->center_y };
    
    fixui_adjust_RECT(wndrect, &old_rect);
    POINT pt;
    fixui_adjust_POINT(&pt, &old_pt);
    this->center_x = pt.x;
    this->center_y = pt.y;

    fixui_pushidentity();
    UIStaticEXA_Render(this);
    fixui_popstate();
    
    this->center_x = old_pt.x;
    this->center_y = old_pt.y;
    *wndrect = old_rect;
}

MAKE_PATCHSET(fixuistaticex)
{
    double ratio = get_frect_aspect_ratio(&game_frect);
    float a = ratio, b = 1.0 / ratio, c = 1.0;
    
    // fix UIStaticEX aspect ratio (e.g. compass at scene)
    memcpy_to_process(0x00442F2C, &c, sizeof(float));
    memcpy_to_process(0x00442F31, &b, sizeof(float));
    memcpy_to_process(0x00442F7E, &c, sizeof(float));
    memcpy_to_process(0x00442F83, &a, sizeof(float));
    INIT_WRAPPER_VFPTR(UIStaticEX_Render_wrapper, 0x0056AE14);
    
    // fix UIStaticEXA aspect ratio (e.g. taiji at splash)
    memcpy_to_process(0x004432BD, &c, sizeof(float));
    memcpy_to_process(0x004432C2, &b, sizeof(float));
    memcpy_to_process(0x0044330F, &c, sizeof(float));
    memcpy_to_process(0x00443314, &a, sizeof(float));
    INIT_WRAPPER_VFPTR(UIStaticEXA_Render_wrapper, 0x0056AE30);
}
