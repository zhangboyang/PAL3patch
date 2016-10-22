#include "common.h"

static int ccbui_dstrect_type;

static MAKE_ASMPATCH(fixattacksequen_back)
{
    struct UIWnd *pwnd = TOPTR(M_DWORD(R_EBP + 0x48));
    struct uiwnd_ptag ptag = MAKE_PTAG(SF_COMBAT, PTR_GAMERECT_ORIGINAL, ccbui_dstrect_type, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pwnd, ptag);
    R_EDX = M_DWORD(0x00570378); // oldcode
}

MAKE_PATCHSET(fixcombatui)
{
    scalefactor_table[SF_COMBAT] = 1.5; // FIXME: let user configure
    ccbui_dstrect_type = PTR_GAMERECT_43;
    
    INIT_ASMPATCH(fixattacksequen_back, 0x005176F5, 6, "\x8B\x15\x78\x03\x57\x00");
}
