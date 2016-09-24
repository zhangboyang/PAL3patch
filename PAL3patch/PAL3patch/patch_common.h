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
        MAKE_PATCHSET(fixloadingfrm);
        MAKE_PATCHSET(fixcombatui);
        MAKE_PATCHSET(fixroledialog);
        MAKE_PATCHSET(fixgameover);
    MAKE_PATCHSET(replacetexture);

#endif
