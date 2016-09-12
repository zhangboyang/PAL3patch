#include "common.h"

// transform functions

void set_rect(RECT *rect, int left, int top, int right, int bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

void move_rect(RECT *rect, int lr_diff, int tb_diff)
{
    rect->left += lr_diff;
    rect->right += lr_diff;
    rect->top += tb_diff;
    rect->bottom += tb_diff;
}
void locate_rect(RECT *rect, int lr, int tb, int lrflags, int tbflags)
{
    int width, height;
    get_rect_size(rect, &width, &height);
    switch (lrflags) {
        case TR_NONE:
            break;
        case TR_LOW:
            rect->left = lr;
            rect->right = rect->left + width;
            break;
        case TR_HIGH:
            rect->right = lr;
            rect->left = rect->right - width;
            break;
        default:
            fail("unknown left-right locate method: %d", lrflags);
    }
    switch (tbflags) {
        case TR_NONE:
            break;
        case TR_LOW:
            rect->top = tb;
            rect->bottom = rect->top + height;
            break;
        case TR_HIGH:
            rect->bottom = tb;
            rect->top = rect->bottom - height;
            break;
        default:
            fail("unknown top-bottom locate method: %d", tbflags);
    }
}
void adjust_rect(RECT *rect, int new_width, int new_height, int lrflags, int tbflags)
{
    switch (lrflags) {
        case TR_NONE:
            break;
        case TR_LOW:
            rect->right = rect->left + new_width;
            break;
        case TR_HIGH:
            rect->left = rect->right - new_width;
            break;
        default:
            fail("unknown width adjust method: %d", lrflags);
    }
    switch (tbflags) {
        case TR_NONE:
            break;
        case TR_LOW:
            rect->bottom = rect->top + new_height;
            break;
        case TR_HIGH:
            rect->top = rect->bottom - new_height;
            break;
        default:
            fail("unknown height adjust method: %d", tbflags);
    }
}
void extend_rect(RECT *rect, int width_diff, int height_diff, int lrflags, int tbflags)
{
    int width, height;
    get_rect_size(rect, &width, &height);
    adjust_rect(rect, width + width_diff, height + height_diff, lrflags, tbflags);
}
void get_rect_size(RECT *rect, int *width, int *height)
{
    if (width) *width = get_rect_width(rect);
    if (height) *height = get_rect_height(rect);
}
void set_rect_size(RECT *rect, int width, int height)
{
    adjust_rect(rect, width, height, TR_LOW, TR_LOW);
}
void scale_rect(RECT *rect, int lrflags, int tbflags)
{
    int width, height;
    get_rect_size(rect, &width, &height);
    width *= scalefactor;
    height *= scalefactor;
    adjust_rect(rect, width, height, lrflags, tbflags);
}
int get_trflag_direction(int flags)
{
    switch (flags) {
        case TR_NONE:
            return TR_NONE;
        case TR_CENTER:
        case TR_LOW:
        case TR_CENTERLOW:
        case TR_ALIGNLOW:
        case TR_SHIFTLOW:
        case TR_SHIFTLOWSCALE:
            return TR_LOW;
        case TR_HIGH:
        case TR_CENTERHIGH:
        case TR_ALIGNHIGH:
        case TR_SHIFTHIGH:
        case TR_SHIFTHIGHSCALE:
            return TR_HIGH;
        default: fail("unknown transform method: %d\n", flags);
    }
}

/*
    transform segment
    return transformed position
    
    parameters:
        <=============total==============>
               <=====total_org====>
        |shift|<=====total_43=====>|shift|
              |  <--length-->      |
              |  ^position         |
    possible positions:
        LOW
                                      HIGH
              CENTER_LOW
                         CENTER_HIGH
                     CENTER
           ^ALIGN_LOW
                       ALIGN_HIGH^
                 ^SHIFT_LOW
                 ^SHIFT_LOW_SCALE
            SHIFT_HIGH_SCALE^
                  SHIFT_HIGH^
*/
static int transform_segment(int position, int length, int flags, int total_org, int total, int total_43, int shift)
{
    switch (flags) {
        case TR_NONE:            return position;
        case TR_LOW:             return 0;
        case TR_HIGH:            return total - length;

        case TR_CENTER:          return (total - length) / 2;
        case TR_CENTERLOW:       return shift;
        case TR_CENTERHIGH:      return shift + total_43 - length;

        case TR_ALIGNLOW:        return position;
        case TR_ALIGNHIGH:       return total - (total_org - (position + length)) - length;
        
        case TR_SHIFTLOW:        return shift + position;
        case TR_SHIFTHIGH:       return shift + total_43 - (total_org - (position + length)) - length;
        
        case TR_SHIFTLOWSCALE:   return shift + scalefactor * position;
        case TR_SHIFTHIGHSCALE:  return shift + total_43 - scalefactor * (total_org - (position + length)) - length;
        
        default: fail("unknown segment transform method: %d\n", flags);
    }
}

static int transform_segment_lr(int position, int length, int flags)
{
    return transform_segment(position, length, flags, GAME_WIDTH_ORG, game_width, game_width_43, width_shift);
}
static int transform_segment_tb(int position, int length, int flags)
{
    return transform_segment(position, length, flags, GAME_HEIGHT_ORG, game_height, game_height_43, height_shift);
}
static int transform_point_lr(int point, int flags)
{
    return transform_segment_lr(point, 0, flags);
}
static int transform_point_tb(int point, int flags)
{
    return transform_segment_tb(point, 0, flags);
}
void transform_point(int *lr, int *tb, int lrflags, int tbflags)
{
    if (lr) *lr = transform_point_lr(*lr, lrflags);
    if (tb) *tb = transform_point_tb(*tb, tbflags);
}




/*
    transform rect
    lrflags: Left and Right flags
    tbflags: Top and Bottom flags
    whflags: Width and Height flags (accept TR_SCALE only)
*/
void transform_rect(RECT *rect, int lrflags, int tbflags, int whflags)
{
    // transform length
    switch (whflags) {
        case TR_NONE:
            break;
        case TR_SCALE:
            scale_rect(rect, get_trflag_direction(lrflags), get_trflag_direction(tbflags));
            break;
        default:
            fail("unknown length transform method: %d", whflags);
    }
    
    // calc parameters
    int left = rect->left, top = rect->top;
    int width, height;
    get_rect_size(rect, &width, &height);

    // transform position
    left = transform_segment_lr(left, width, lrflags);
    top = transform_segment_tb(top, height, tbflags);
    
    // finally, set rect
    set_rect(rect, left, top, left + width, top + height);
}




MAKE_PATCHSET(fixui)
{
}
