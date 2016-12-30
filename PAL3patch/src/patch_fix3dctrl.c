#include "common.h"

struct ui3dctrl_origininfo {
    // NOTE: sizeof this struct should smaller than or equal to sizeof RECT
    int x;
    int y;
    int from_y;
    int flag;
};
#define oinfo(this) (*(struct ui3dctrl_origininfo *)(&this->baseclass.m_rect))

struct ui3dctrl_orthoinfo {
    float m_orthosize_sv;
};
static void ui3dctrl_updatecamera(struct UI3DCtrl *this, struct ui3dctrl_orthoinfo *sv)
{
    double ui_orthofactor = (game_scalefactor / fs->len_factor);
    
    POINT pt = { oinfo(this).x, oinfo(this).y };
    fixui_adjust_POINT(&pt, &pt);
    pt.y = gby2y(y2gby(pt.y) * (game_height * 4.0 / (game_width * 3.0)));
    //GetCursorPos(&pt); ScreenToClient(gfxdrvinfo.hgfxwnd, &pt);
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



static int __fastcall UI3DCtrl_Update_wrapper(struct UI3DCtrl *this, int dummy, float deltatime, int haveinput)
{
    //struct ui3dctrl_orthoinfo sv;
    //ui3dctrl_updatecamera(this, &sv);
    fixui_pushidentity();
    int ret = UI3DCtrl_Update(this, deltatime, haveinput);
    fixui_popstate();
    //ui3dctrl_restoreortho(this, &sv);
    return ret;
}

static void __fastcall UI3DCtrl_Render_wrapper(struct UI3DCtrl *this, int dummy)
{
    struct ui3dctrl_orthoinfo sv;
    ui3dctrl_updatecamera(this, &sv);
    UI3DCtrl_Render(this);
    ui3dctrl_restoreortho(this, &sv);
}

static void __fastcall UI3DCtrl_SetOriginPt_XY_wrapper(struct UI3DCtrl *this, int dummy, int x, int y)
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
static void __fastcall UI3DCtrl_SetOriginPt_XYFromY_wrapper(struct UI3DCtrl *this, int dummy, int x, int y, int from_y)
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
        0x0047F7F2,
        0x0047F864,
        0x004AC450,
        0x004AC4D7,
        0x00525E3D,
        0x00525EAA,
        0x00525FA7,
        0x005288F9,
        0x00528B19,
    });
    INIT_WRAPPER_CALL(UI3DCtrl_SetOriginPt_XYFromY_wrapper, {
        0x0045E6D9,
        0x0046C4CA,
        0x0046EB65,
        0x00470477,
        0x004729C0,
        0x00478114,
        0x0047A25F,
        0x0048E4B5,
        0x00494E76,
        0x00495038,
        0x004951FB,
        0x004953D8,
        0x0049559B,
        0x0049575E,
        0x00495AB0,
        0x00496D30,
        0x004984EB,
        0x0049B33B,
        0x0049B365,
        0x0049F91F,
        0x004A0EBF,
    });

    INIT_WRAPPER_VFPTR(UI3DCtrl_Render_wrapper, 0x0056AC24);
    INIT_WRAPPER_VFPTR(UI3DCtrl_Update_wrapper, 0x0056AC28);
}
