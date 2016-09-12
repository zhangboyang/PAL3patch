#include "common.h"


static int borderflag; // should we leave space for making UI 4:3
static int sl_flag, sh_flag; // ShiftLow flag, ShiftHigh flag, configured by 'borderflag'

static MAKE_ASMPATCH(fixbigwinlose)
{
    RECT *rect = TOPTR(R_ESP + 0x8);
    set_rect(rect, 150, -50, 406, 462);
    int old_height = get_rect_height(rect);
    adjust_rect(rect, 0, 350, TR_NONE, TR_HIGH);
    transform_rect(rect, (borderflag ? TR_SHIFTLOWSCALE : TR_ALIGNLOW), (borderflag ? TR_SHIFTLOWSCALE : TR_ALIGNLOW), TR_NONE);
    adjust_rect(rect, 0, old_height, TR_NONE, TR_HIGH);
}

static MAKE_ASMPATCH(fixresultback)
{
    RECT *rect = TOPTR(R_ESP + 0x14);
    rect->bottom = 256; // oldcode
    transform_rect(rect, sl_flag, sl_flag, TR_NONE);
}

static MAKE_ASMPATCH(fixresultpanel_lvup)
{
    RECT *rect = TOPTR(R_ESP + 0x14);
    set_rect(rect, 284, 268, 516, 332);
    transform_rect(rect, TR_CENTER, TR_CENTER, TR_NONE);
}
static MAKE_ASMPATCH(fixresultpanel_magiclvup)
{
    RECT *rect = TOPTR(R_ESP + 0x14);
    set_rect(rect, 249, 263, 551, 337);
    transform_rect(rect, TR_CENTER, TR_CENTER, TR_NONE);
}
static MAKE_ASMPATCH(fixresultpanel_getitem)
{
    RECT *rect = TOPTR(R_ESP + 0x14);
    set_rect(rect, 221, 255, 579, 345);
    transform_rect(rect, TR_CENTER, TR_CENTER, TR_NONE);
}


static void fixstatuspanel()
{
    unsigned array_locations[] = {
        0x0056A6EC,
        0x0056B070,
        0x0056FB90,
        0x0056FBB8,
        0x0056FC38,
        0x0056FC60,
        0x0056FC98,
        0x0056FCC8,
        0x0056FD14,
        0x0056FD58,
        0x0056FD8C,
        0x0056FF88,
        0x0056FFF8,
        0x0057002C,
        0x00570058,
        0x00570178,
    };
    unsigned array_locations_cnt = sizeof(array_locations) / sizeof(array_locations[0]);

    int array[2][4] = {
        {   0, 207, 403, 600 },
        { 491, 491, 491, 491 },
    };
    
    // do transform
    int i;
    for (i = 0; i < 4; i++) {
        transform_point(&array[0][i], &array[1][i], sl_flag, sh_flag);
    }
    
    // write to process
    for (i = 0; i < array_locations_cnt; i++) {
        memcpy_to_process(array_locations[i], array, sizeof(array));
    }
}
static MAKE_ASMPATCH(fix_statuspanel_particles)
{
    int left = 25, top = 570;
    transform_point(&left, &top, sl_flag, sh_flag);
    PUSH_DWORD(top);
    PUSH_DWORD(left);
}


static MAKE_ASMPATCH(fixcontrol_back)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 10, 225, 266, 481);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_skill)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 5, 237, 261, 493);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_magic)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 2, 278, 251, 484);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_item)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 6, 278, 255, 484);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_ai)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 14, 316, 172, 474);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_lineup)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 9, 232, 239, 482);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixcontrol_protect)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 10, 271, 240, 471);
    transform_rect(rect, sl_flag, sh_flag, TR_NONE);
}
static void fixcontol_lineup_head()
{
    int left_array[6], top_array[6];
    unsigned left_array_addr = 0x00570250;
    unsigned top_array_addr = 0x0057026C;
    memcpy_from_process(left_array, left_array_addr, sizeof(left_array));
    memcpy_from_process(top_array, top_array_addr, sizeof(top_array));
    int i;
    for (i = 0; i < 6; i++) {
        transform_point(&left_array[i], &top_array[i], sl_flag, sh_flag);
    }
    memcpy_to_process(left_array_addr, left_array, sizeof(left_array));
    memcpy_to_process(top_array_addr, top_array, sizeof(top_array));
}



static MAKE_ASMPATCH(fixattacksequen_particles)
{
    int left = 691, top = 36;
    transform_point(&left, &top, sh_flag, sl_flag);
    PUSH_DWORD(top);
    PUSH_DWORD(left);
}
static MAKE_ASMPATCH(fixattacksequen_back)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 483, 10, 803, 42);
    transform_rect(rect, sh_flag, sl_flag, TR_NONE);
}
static MAKE_ASMPATCH(fixattacksequen_faceinit)
{
    RECT *rect = TOPTR(R_ESP + 0x10);
    set_rect(rect, 480, 8, 544, 72);
    transform_rect(rect, sh_flag, sl_flag, TR_NONE);
}
#define sub_445D20(this, left, top) THISCALL_WRAPPER(MAKE_THISCALL_FUNCPTR(0x00445D20, int, void *, int, int), this, left, top)
static int __fastcall fixattacksequen_faceposition_hook(void *this, int dummy, int left, int top)
{
    transform_point(&left, &top, sh_flag, sl_flag);
    return sub_445D20(this, left, top);
}



static MAKE_ASMPATCH(fixtrickname)
{
    RECT *rect = TOPTR(R_ESP + 0x0C);
    set_rect(rect, 300, 30, 461, 87);
    transform_rect(rect, TR_CENTER, sl_flag, TR_NONE);
}


MAKE_PATCHSET(fixcombatui)
{
    borderflag = flag != 2;
    sl_flag = borderflag ? TR_SHIFTLOW : TR_ALIGNLOW;
    sh_flag = borderflag ? TR_SHIFTHIGH : TR_ALIGNHIGH;
    
    // fix 'bigwin.tga' and 'biglose.tga'
    INIT_ASMPATCH(fixbigwinlose, 0x00516979, 0x20, "\xC7\x44\x24\x08\x96\x00\x00\x00\xC7\x44\x24\x10\x96\x01\x00\x00\xC7\x44\x24\x0C\xCE\xFF\xFF\xFF\xC7\x44\x24\x14\xCE\x01\x00\x00");
    INIT_ASMPATCH(fixbigwinlose, 0x005169DB, 0x20, "\xC7\x44\x24\x08\x96\x00\x00\x00\xC7\x44\x24\x10\x96\x01\x00\x00\xC7\x44\x24\x0C\xCE\xFF\xFF\xFF\xC7\x44\x24\x14\xCE\x01\x00\x00");
    
    // fix result 'back.tga'
    INIT_ASMPATCH(fixresultback, 0x00517DAC, 0x8, "\xC7\x44\x24\x20\x00\x01\x00\x00");
    
    // fix result panel
    INIT_ASMPATCH(fixresultpanel_lvup, 0x00517E41, 0x20, "\xC7\x44\x24\x14\x1C\x01\x00\x00\xC7\x44\x24\x1C\x04\x02\x00\x00\xC7\x44\x24\x18\x0C\x01\x00\x00\xC7\x44\x24\x20\x4C\x01\x00\x00");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x00517EC6, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x00517F37, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x00517FA8, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x00518019, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x0051808A, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_magiclvup, 0x005180FB, 0x18, "\xC7\x44\x24\x14\xF9\x00\x00\x00\xC7\x44\x24\x1C\x27\x02\x00\x00\x89\x5C\x24\x18\x89\x6C\x24\x20");
    INIT_ASMPATCH(fixresultpanel_getitem, 0x0051816C, 0x20, "\xC7\x44\x24\x14\xDD\x00\x00\x00\xC7\x44\x24\x1C\x43\x02\x00\x00\xC7\x44\x24\x18\xFF\x00\x00\x00\xC7\x44\x24\x20\x59\x01\x00\x00");
    
    // fix status panel
    fixstatuspanel();
    INIT_ASMPATCH(fix_statuspanel_particles, 0x004F85F5, 0x7, "\x68\x3A\x02\x00\x00\x6A\x19");
    INIT_ASMPATCH(fix_statuspanel_particles, 0x004F862F, 0x7, "\x68\x3A\x02\x00\x00\x6A\x19");
    
    // fix contol
    INIT_ASMPATCH(fixcontrol_back, 0x00517A83, 0x20, "\xC7\x44\x24\x10\x0A\x00\x00\x00\xC7\x44\x24\x18\x0A\x01\x00\x00\xC7\x44\x24\x14\xE1\x00\x00\x00\xC7\x44\x24\x1C\xE1\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_skill, 0x00517BD3, 0x20, "\xC7\x44\x24\x10\x05\x00\x00\x00\xC7\x44\x24\x18\x05\x01\x00\x00\xC7\x44\x24\x14\xED\x00\x00\x00\xC7\x44\x24\x1C\xED\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_magic, 0x00517B63, 0x20, "\xC7\x44\x24\x10\x02\x00\x00\x00\xC7\x44\x24\x18\xFB\x00\x00\x00\xC7\x44\x24\x14\x16\x01\x00\x00\xC7\x44\x24\x1C\xE4\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_item, 0x00517AF3, 0x20, "\xC7\x44\x24\x10\x06\x00\x00\x00\xC7\x44\x24\x18\xFF\x00\x00\x00\xC7\x44\x24\x14\x16\x01\x00\x00\xC7\x44\x24\x1C\xE4\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_ai, 0x00517C43, 0x20, "\xC7\x44\x24\x10\x0E\x00\x00\x00\xC7\x44\x24\x18\xAC\x00\x00\x00\xC7\x44\x24\x14\x3C\x01\x00\x00\xC7\x44\x24\x1C\xDA\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_lineup, 0x00517CB3, 0x20, "\xC7\x44\x24\x10\x09\x00\x00\x00\xC7\x44\x24\x18\xEF\x00\x00\x00\xC7\x44\x24\x14\xE8\x00\x00\x00\xC7\x44\x24\x1C\xE2\x01\x00\x00");
    INIT_ASMPATCH(fixcontrol_protect, 0x00517D23, 0x20, "\xC7\x44\x24\x10\x0A\x00\x00\x00\xC7\x44\x24\x18\xF0\x00\x00\x00\xC7\x44\x24\x14\x0F\x01\x00\x00\xC7\x44\x24\x1C\xD7\x01\x00\x00");
    fixcontol_lineup_head();

    // fix attacksequen
    INIT_ASMPATCH(fixattacksequen_particles, 0x004DC4F0, 0x7, "\x6A\x24\x68\xB3\x02\x00\x00");
    INIT_ASMPATCH(fixattacksequen_back, 0x005176E2, 0x8, "\xC7\x44\x24\x1C\x2A\x00\x00\x00");
    INIT_ASMPATCH(fixattacksequen_faceinit, 0x00517706, 0x20, "\xC7\x44\x24\x10\xE0\x01\x00\x00\xC7\x44\x24\x18\x20\x02\x00\x00\xC7\x44\x24\x14\x08\x00\x00\x00\xC7\x44\x24\x1C\x48\x00\x00\x00");
    make_call(0x004DBB67, fixattacksequen_faceposition_hook);
    
    // fix trickname
    INIT_ASMPATCH(fixtrickname, 0x005178CC, 0x8, "\xC7\x44\x24\x18\x57\x00\x00\x00");
    
}
