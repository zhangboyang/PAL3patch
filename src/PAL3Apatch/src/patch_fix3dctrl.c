#include "common.h"

struct ui3dctrl_origininfo {
    // NOTE: sizeof this struct should smaller than or equal to sizeof RECT
    int x;
    int y;
    int from_y;
    int flag;
};
#define oinfo(this) (*(struct ui3dctrl_origininfo *)(&this->m_rect))

struct ui3dctrl_orthoinfo {
    float m_orthosize_sv;
};
static void ui3dctrl_updatecamera(struct UI3DCtrl *this, struct ui3dctrl_orthoinfo *sv)
{
    double ui_orthofactor = (game_scalefactor / fs->len_factor);
    
    POINT pt = { oinfo(this).x, oinfo(this).y };
    fixui_adjust_POINT(&pt, &pt);
    pt.y = floor(gby2y(y2gby(pt.y) * (game_height * 4.0 / (game_width * 3.0))) + eps);
    sv->m_orthosize_sv = this->m_orthosize;
    this->m_camera->OrthoSize = this->m_orthosize * ui_orthofactor;
    this->m_orthosize *= orthofactor * ui_orthofactor;
    
    if (oinfo(this).flag) {
        UI3DCtrl_SetOriginPt_XYFromY(this, pt.x, pt.y, oinfo(this).from_y);
    } else {
        UI3DCtrl_SetOriginPt_XY(this, pt.x, pt.y);
    }
}
static void ui3dctrl_restoreortho(struct UI3DCtrl *this, struct ui3dctrl_orthoinfo *sv)
{
    this->m_orthosize = sv->m_orthosize_sv;
}



static MAKE_THISCALL(int, UI3DCtrl_Update_wrapper, struct UI3DCtrl *this, float deltatime, int haveinput)
{
    //struct ui3dctrl_orthoinfo sv;
    //ui3dctrl_updatecamera(this, &sv);
    fixui_pushidentity();
    int ret = UI3DCtrl_Update(this, deltatime, haveinput);
    fixui_popstate();
    //ui3dctrl_restoreortho(this, &sv);
    return ret;
}

static MAKE_THISCALL(void, UI3DCtrl_Render_wrapper, struct UI3DCtrl *this)
{
    struct ui3dctrl_orthoinfo sv;
    ui3dctrl_updatecamera(this, &sv);
    UI3DCtrl_Render(this);
    ui3dctrl_restoreortho(this, &sv);
}

static MAKE_THISCALL(void, UI3DCtrl_SetOriginPt_XY_wrapper, struct UI3DCtrl *this, int x, int y)
{
    oinfo(this) = (struct ui3dctrl_origininfo) {
        .x = x,
        .y = y,
        .from_y = 0,
        .flag = 0,
    };
    //plog("SAVE: %p %d %d %.10f", this, x, y, this->m_orthosize);
    UI3DCtrl_SetOriginPt_XY(this, x, y);
}
static MAKE_THISCALL(void, UI3DCtrl_SetOriginPt_XYFromY_wrapper, struct UI3DCtrl *this, int x, int y, int from_y)
{
    oinfo(this) = (struct ui3dctrl_origininfo) {
        .x = x,
        .y = y,
        .from_y = from_y,
        .flag = 1,
    };
    //plog("SAVE2: %p %d %d %d %.10f", this, x, y, from_y, this->m_orthosize);
    UI3DCtrl_SetOriginPt_XYFromY(this, x, y, from_y);
}


MAKE_PATCHSET(fix3dctrl)
{
    INIT_WRAPPER_CALL(UI3DCtrl_SetOriginPt_XY_wrapper, {
        0x0047D631,
        0x0047D569,
        0x0047FFF1,
    });
    INIT_WRAPPER_CALL(UI3DCtrl_SetOriginPt_XYFromY_wrapper, {
        0x00466439,
        0x0046CFE3,
        0x0046D3C1,
        0x0046D548,
        0x00470187,
        0x00470BFB,
        0x00470FB3,
        0x00471113,
        0x00471189,
        0x0047124E,
        0x0047130A,
        0x004713C2,
        0x004714CB,
        0x0047158E,
        0x00471622,
        0x0047466E,
        0x00478B8F,
        0x00478F6D,
        0x00479104,
        0x004794C0,
        0x00479620,
        0x00479698,
        0x0047975B,
        0x00479817,
        0x004798CF,
        0x004799DA,
        0x00479A9D,
        0x00479B31,
        0x00491997,
        0x0049216F,
        0x00492568,
        0x0049266B,
        0x004926F7,
        0x00499699,
        0x0049A1BF,
        0x0049A577,
        0x0049A6D7,
        0x0049A74D,
        0x0049A812,
        0x0049A8CE,
        0x0049A986,
        0x0049AA8F,
        0x0049AB52,
        0x0049ABE6,
        0x0049E7AE,
    });

    INIT_WRAPPER_VFPTR(UI3DCtrl_Render_wrapper, 0x00558C04);
    INIT_WRAPPER_VFPTR(UI3DCtrl_Update_wrapper, 0x00558C08);
}
