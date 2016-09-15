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
    extern int game_width_43, game_height_43;
    extern int width_shift, height_shift;
    extern double scalefactor;
    MAKE_PATCHSET(windowed);
    MAKE_PATCHSET(fixfov);
    MAKE_PATCHSET(nolockablebackbuffer);
    MAKE_PATCHSET(fixreset);
    MAKE_PATCHSET(fixui);
        enum transform_type {
            TR_NONE,
            TR_LOW,
            TR_HIGH,
            TR_SCALE,
            TR_CENTER,
            TR_CENTERLOW,
            TR_CENTERHIGH,
            TR_ALIGNLOW,
            TR_ALIGNHIGH,
            TR_SHIFTLOW,
            TR_SHIFTHIGH,
            TR_SHIFTLOWSCALE,
            TR_SHIFTHIGHSCALE,
        };
        extern void set_rect(RECT *rect, int left, int top, int right, int bottom);
        extern void move_rect(RECT *rect, int lr_diff, int tb_diff);
        extern void locate_rect(RECT *rect, int lr, int tb, int lrflags, int tbflags);
        extern void adjust_rect(RECT *rect, int new_width, int new_height, int lrflags, int tbflags);
        extern void extend_rect(RECT *rect, int width_diff, int height_diff, int lrflags, int tbflags);
        #define get_rect_width(rect) ((rect)->right - (rect)->left)
        #define get_rect_height(rect) ((rect)->bottom - (rect)->top)
        extern void get_rect_size(RECT *rect, int *width, int *height);
        extern void set_rect_size(RECT *rect, int width, int height);
        extern void scale_rect(RECT *rect, int lrflags, int rbflags);
        extern int get_trflag_direction(int flags);
        extern void transform_point(int *lr, int *tb, int lrflags, int tbflags);
        extern void transform_rect(RECT *rect, int lrflags, int tbflags, int whflags);
        MAKE_PATCHSET(fixloadingfrm);
        MAKE_PATCHSET(fixcombatui);
        MAKE_PATCHSET(fixroledialog);
        MAKE_PATCHSET(fixgameover);
        MAKE_PATCHSET(replacetexture);

#endif
