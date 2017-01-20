#include "common.h"

static void fix_FVF()
{
    // fix FVF problem in combat system (CCBStageEffect::GeomFlashEffect_t)
    add_effect_hook("geom_diffuse_t.gbf", "FVF = XYZ|DIFFUSE|TEX1;", "FVF = XYZ|NORMAL|TEX1;");

    // fix FVF problem in YeTan mini-game (ui_notex)
    add_effect_hook("ui_notex.gbf", "FVF          = XYZ | DIFFUSE | TEX1;", "FVF = XYZ|DIFFUSE;");
}

MAKE_PATCHSET(fixeffect)
{
    fix_FVF();
}
