#ifndef PAL3PATCH_PATCH_COMMON_H
#define PAL3PATCH_PATCH_COMMON_H

#define GAME_WIDTH_ORG 800
#define GAME_HEIGHT_ORG 600

// all patchs
MAKE_PATCHSET(testcombat);
MAKE_PATCHSET(cdpatch);
MAKE_PATCHSET(regredirect);
MAKE_PATCHSET(disablekbdhook);
MAKE_PATCHSET(depcompatible);
MAKE_PATCHSET(setlocale);
MAKE_PATCHSET(dpiawareness);
MAKE_PATCHSET(powersave);
MAKE_PATCHSET(timerresolution);
MAKE_PATCHSET(fixmemfree);

MAKE_PATCHSET(graphicspatch);
    extern int game_width, game_height;
    extern fRECT game_frect, game_frect_43, game_frect_original;
    extern double game_scalefactor;
    MAKE_PATCHSET(windowed);
    MAKE_PATCHSET(fixfov);
    MAKE_PATCHSET(nolockablebackbuffer);
    MAKE_PATCHSET(fixreset);
    MAKE_PATCHSET(fixui);
        enum UIadjust_screen_type {
            UI_ADJUST_FULLSCREEN,
            UI_ADJUST_BORDERED,
        };
        enum UIadjust_mouse_type {
            UI_MOUSE_NORMAL,
            UI_MOUSE_VIRTUALIZED,
        };
        extern void UIadjust_setstate(int screen_type, int lr_method, int tb_method, double len_factor);
        extern void UIadjust_gbUIQuad(struct gbUIQuad *out_uiquad, const struct gbUIQuad *uiquad);
        extern void UIadjust_RECT(RECT *out_rect, const RECT *rect);
        MAKE_PATCHSET(fixloadingfrm);
        MAKE_PATCHSET(fixcombatui);
        MAKE_PATCHSET(fixroledialog);
        MAKE_PATCHSET(fixgameover);
    MAKE_PATCHSET(replacetexture);

#endif
