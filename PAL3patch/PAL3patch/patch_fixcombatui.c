#include "common.h"

#define CB_PUSHSTATE(lr, tb) fixui_pushstate(get_ptag_frect(PTR_GAMERECT_ORIGINAL), get_ptag_frect(ccbui_dstrect_type), (lr), (tb), cb_scalefactor)
#define CB_POPSTATE fixui_popstate
#define CB_PTAG(lr, tb) MAKE_PTAG(SF_COMBAT, PTR_GAMERECT_ORIGINAL, ccbui_dstrect_type, (lr), (tb))

static int ccbui_dstrect_type;


static MAKE_ASMPATCH(fixattacksequen_particles)
{
    // FIXME: fix UIQuad size in C2DSpark::Render()
    //        note: mouse effect when combat win may also use C2DSpark
    POINT pt = {691, 36};
    CB_PUSHSTATE(TR_HIGH, TR_LOW);
    fixui_adjust_POINT(&pt, &pt);
    CB_POPSTATE();
    PUSH_DWORD(pt.y);
    PUSH_DWORD(pt.x);
}
static MAKE_ASMPATCH(fixattacksequen_back)
{
    set_uiwnd_ptag(PWND(R_EBP + 0x48), CB_PTAG(TR_HIGH, TR_LOW));
    R_EDX = M_DWORD(0x00570378); // oldcode
}

static MAKE_ASMPATCH(fixtrickname)
{
    set_uiwnd_ptag(PWND(R_ESI + 0x430), CB_PTAG(TR_CENTER, TR_LOW));
    R_ECX = M_DWORD(0x005703F4); // oldcode
}
static MAKE_ASMPATCH(fixtrickname2)
{
    set_uiwnd_ptag(PWND(R_ESI + 0x434), CB_PTAG(TR_CENTER, TR_LOW));
    R_ECX = M_DWORD(0x005703F8); // oldcode
}


MAKE_PATCHSET(fixcombatui)
{
    cb_scalefactor = str2scalefactor(get_string_from_configfile("fixcombatui_scalefactor"));
    ccbui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixcombatui_scaletype"));

    // fix attacksequen
    INIT_ASMPATCH(fixattacksequen_particles, 0x004DC4F0, 0x7, "\x6A\x24\x68\xB3\x02\x00\x00");
    INIT_ASMPATCH(fixattacksequen_back, 0x005176F5, 6, "\x8B\x15\x78\x03\x57\x00");
    
    // fix trickname
    INIT_ASMPATCH(fixtrickname, 0x005178E0, 6, "\x8B\x0D\xF4\x03\x57\x00");
    INIT_ASMPATCH(fixtrickname2, 0x00517933, 6, "\x8B\x0D\xF8\x03\x57\x00");
}
