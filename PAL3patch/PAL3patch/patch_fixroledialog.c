#include "common.h"
#include "gbengine.h"


#define FACE_LRSHIFT 18
#define FACE_TBSHIFT 9

static RECT dialog_org, dialog_target;
static int left_diff, top_diff, width_diff, height_diff;

static void __stdcall get_arrow_rect(RECT *rect, BOOL flag)
{
    // flag: arrow position  0->left  1->right
    set_rect(rect, (flag ? 760 : 18), 552, (flag ? 792 : 50), 584);
    
    // combine 3 move_rect() in 1
    move_rect(rect, left_diff + (flag ? width_diff : 0), top_diff + height_diff);
    /*move_rect(rect, left_diff, top_diff);
    move_rect(rect, 0, height_diff);
    if (flag) move_rect(rect, width_diff, 0);*/
}

static MAKE_ASMPATCH(fixback)
{
    RECT *rect = TOPTR(R_EDI);
    *rect = dialog_target;
}
static MAKE_ASMPATCH(fixtimeclose)
{
    RECT *rect = TOPTR(R_ESP + 0x18);
    rect->top = 580; // oldcode
    move_rect(rect, left_diff, top_diff + height_diff);
}
static MAKE_ASMPATCH(fixtimeprogress)
{
    RECT *rect = TOPTR(R_ESP + 0x18);
    rect->top = 568; // oldcode
    move_rect(rect, left_diff, top_diff + height_diff);
}


#define MAKE_UIWND_SETRECT_WRAPPER(name) void __fastcall name(struct UIWnd *this, int dummy, RECT *rect)

static MAKE_UIWND_SETRECT_WRAPPER(adjust_noface_textfield)
{
    RECT new_rect = *rect;
    extend_rect(&new_rect, width_diff, height_diff, TR_HIGH, TR_LOW);
    UIWnd_SetRect(this, &new_rect);
}
static MAKE_UIWND_SETRECT_WRAPPER(adjust_leftface_textfield)
{
    RECT new_rect = *rect;
    extend_rect(&new_rect, width_diff - FACE_LRSHIFT, height_diff, TR_HIGH, TR_LOW);
    UIWnd_SetRect(this, &new_rect);
}
static MAKE_UIWND_SETRECT_WRAPPER(adjust_rightface_textfield)
{
    RECT new_rect = *rect;
    extend_rect(&new_rect, width_diff - FACE_LRSHIFT, height_diff, TR_LOW, TR_LOW);
    UIWnd_SetRect(this, &new_rect);
}
static MAKE_UIWND_SETRECT_WRAPPER(move_rightface_arrow)
{
    RECT new_rect = *rect;
    move_rect(&new_rect, width_diff, 0);
    move_rect(&new_rect, -FACE_LRSHIFT, -FACE_TBSHIFT);
    UIWnd_SetRect(this, &new_rect);
}
static MAKE_UIWND_SETRECT_WRAPPER(leftface)
{
    RECT new_rect = *rect;
    locate_rect(&new_rect, dialog_target.left, dialog_target.bottom, TR_LOW, TR_HIGH);
    move_rect(&new_rect, FACE_LRSHIFT, -FACE_TBSHIFT);
    UIWnd_SetRect(this, &new_rect);
}
static MAKE_UIWND_SETRECT_WRAPPER(rightface)
{
    RECT new_rect = *rect;
    locate_rect(&new_rect, dialog_target.right, dialog_target.bottom, TR_HIGH, TR_HIGH);
    move_rect(&new_rect, -(2 * 7 + 4), -8);
    UIWnd_SetRect(this, &new_rect);
}

MAKE_PATCHSET(fixroledialog)
{
    set_rect(&dialog_org, 0, 600 - 150, 800, 594);
    
    int dialog_target_width = get_int_from_configfile("roledialogwidth");
    if (dialog_target_width > game_width) dialog_target_width = game_width;
    
    dialog_target = dialog_org;
    if (flag != 2) {
        transform_rect(&dialog_target, TR_NONE, TR_SHIFTLOWSCALE, TR_NONE);
    } else {
        transform_rect(&dialog_target, TR_NONE, TR_CENTERHIGH, TR_NONE);
        move_rect(&dialog_target, 0, -6); // 600 - 594 = 6
    }
    adjust_rect(&dialog_target, dialog_target_width, 0, TR_LOW, TR_NONE);
    transform_rect(&dialog_target, TR_CENTER, TR_NONE, TR_NONE);
    
    // calc parameters
    left_diff = dialog_target.left - dialog_org.left;
    top_diff = dialog_target.top - dialog_org.top;
    width_diff = get_rect_width(&dialog_target) - get_rect_width(&dialog_org);
    height_diff = get_rect_height(&dialog_target) - get_rect_height(&dialog_org);
    
    // hook get_arrow_rect();
    make_jmp(0x004517A0, get_arrow_rect);
    
    // fix UIRoleDialog::Create()
    INIT_ASMPATCH(fixback, 0x00450FBD, 0x7, "\xC7\x47\x0C\x52\x02\x00\x00");
    INIT_ASMPATCH(fixtimeclose, 0x0045111A, 0x8, "\xC7\x44\x24\x1C\x44\x02\x00\x00");
    INIT_ASMPATCH(fixtimeprogress, 0x0045116B, 0x8, "\xC7\x44\x24\x1C\x38\x02\x00\x00");
    
    // fix UIRoleDialog::SetFace()
    make_call(0x00451408, adjust_noface_textfield); // noface, textfield
    make_call(0x004514BA, adjust_leftface_textfield); // leftface, textfield
    make_call(0x0045155F, adjust_rightface_textfield); // rightface, textfield
    make_call(0x0045158A, move_rightface_arrow); // rightface, arrow
    make_call(0x00451461, leftface); // leftface, faceimage
    make_call(0x004514F4, rightface); // rightface, faceimage   
}
