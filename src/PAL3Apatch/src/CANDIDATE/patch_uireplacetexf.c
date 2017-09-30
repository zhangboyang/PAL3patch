#include "common.h"

MAKE_PATCHSET(uireplacetexf)
{
    // fix ui_TL
    add_effect_hook("ui_TL.gbf", "MinFilter[0] = Point;", "MinFilter[0] = LINEAR;");
    add_effect_hook("ui_TL.gbf", "MagFilter[0] = Point;", "MagFilter[0] = LINEAR;");

    // fix ui_tex_color
    add_effect_hook("ui_tex_color.gbf", "MinFilter[0] = POINT;", "MinFilter[0] = LINEAR;");
    add_effect_hook("ui_tex_color.gbf", "MagFilter[0] = POINT;", "MagFilter[0] = LINEAR;");
}
