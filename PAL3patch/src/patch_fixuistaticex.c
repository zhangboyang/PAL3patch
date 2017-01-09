#include "common.h"

static void __fastcall UIStaticEXA_Render_wrapper(struct UIStaticEXA *this, int dummy)
{
    RECT *wndrect = &this->baseclass.baseclass.baseclass.m_rect;
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
    
    // fix UIStaticEXA aspect ratio (e.g. taiji at splash)
    memcpy_to_process(0x004432BD, &c, sizeof(float));
    memcpy_to_process(0x004432C2, &b, sizeof(float));
    memcpy_to_process(0x0044330F, &c, sizeof(float));
    memcpy_to_process(0x00443314, &a, sizeof(float));
    INIT_WRAPPER_VFPTR(UIStaticEXA_Render_wrapper, 0x0056AE30);
}
