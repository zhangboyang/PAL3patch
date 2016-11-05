#include "common.h"

#define CB_POPSTATE fixui_popstate

#define CB_PUSHSTATE(lr, tb) fixui_pushstate(     &game_frect_original , get_ptag_frect(ccbui_dstrect_type), (lr), (tb), cb_scalefactor)
#define CB_PTAG(lr, tb)      MAKE_PTAG(SF_COMBAT, PTR_GAMERECT_ORIGINAL, ccbui_dstrect_type                , (lr), (tb))

static int ccbui_dstrect_type;

// patch C2DSpark, only size is changed, position is not changed
// FIXME: there may be better ways to do the same job using transform framework
static void __fastcall C2DSpark_Render_wrapper(struct C2DSpark *this, int dummy)
{
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE, TR_SCALE, cb_scalefactor);
    C2DSpark_Render(this);
    fixui_popstate();
}
static bool __fastcall C2DSpark_CreateSingle_wrapper(struct C2DSpark *this, int dummy, struct C2DSpark_tagSpark *pSpark)
{
    struct C2DSpark_tagSpark tmp = *pSpark;
    tmp.fVx *= cb_scalefactor; tmp.fAx *= cb_scalefactor;
    tmp.fVy *= cb_scalefactor; tmp.fAy *= cb_scalefactor;
    return C2DSpark_CreateSingle(this, &tmp);
}
static void __fastcall C2DSpark_CreateStars_wrapper(struct C2DSpark *this, int dummy, int x, int y, int nWidth, float fStarSize)
{
    C2DSpark_CreateStars(this, x, y, nWidth * cb_scalefactor, fStarSize);
}
static void patch_c2dspark()
{
    // fix random range
    INIT_WRAPPER_CALL(C2DSpark_CreateStars_wrapper, {
        0x004F85FC,
        0x004F8636,
    });
    
    // fix initial velocity and acceleration
    INIT_WRAPPER_CALL(C2DSpark_CreateSingle_wrapper, {
        0x004D618F,
        0x004D6011,
        0x004D6484,
    });
    
    // fix spark size
    INIT_WRAPPER_CALL(C2DSpark_Render_wrapper, { 0x00513141 });
}

// fix c2dspark positions
static MAKE_ASMPATCH(fixattacksequen_c2dspark)
{
    POINT pt = {691, 36};
    CB_PUSHSTATE(TR_HIGH, TR_LOW);
    fixui_adjust_POINT(&pt, &pt);
    CB_POPSTATE();
    PUSH_DWORD(pt.y);
    PUSH_DWORD(pt.x);
}
static MAKE_ASMPATCH(fix_statuspanel_c2dspark)
{
    POINT pt = {25, 570};
    CB_PUSHSTATE(TR_LOW, TR_HIGH);
    fixui_adjust_POINT(&pt, &pt);
    CB_POPSTATE();
    PUSH_DWORD(pt.y);
    PUSH_DWORD(pt.x);
}


// fix others
static bool __fastcall CCBUI_Create_wrapper(struct CCBUI *this, int dummy)
{
    if (!CCBUI_Create(this)) return false;
    
    int i;
    struct uiwnd_ptag ptag;
    
    // fix rolestate panel
    ptag = CB_PTAG(TR_LOW, TR_HIGH);
    set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateSword, ptag);
    for (i = 0; i < 4; i++) {
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStatePanel[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateFace[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateFaceName[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateAttackInc[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateAttackDec[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateDefenceInc[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateDefenceDec[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateLuckInc[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateLuckDec[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateSpeedInc[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateSpeedDec[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateStable[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateBlank[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateForbid[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateSleep[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateChaos[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateMad[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateMirror[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateWall[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateBound[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateHermit[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateImmunity[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateHP[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateGP[i], ptag);
        set_uiwnd_ptag((struct UIWnd *) this->m_pRoleStateMP[i], ptag);
    }
    
    // fix trickname
    ptag = CB_PTAG(TR_CENTER, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) this->m_pTrickName, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pTrickName2, ptag);
    
    // fix attacksequen
    ptag = CB_PTAG(TR_HIGH, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) this->m_pAttackSequenBack, ptag);
    for (i = 0; i < 11; i++) {
        set_uiwnd_ptag((struct UIWnd *) this->m_pAttackSequenFace[i], ptag);
    }
    
    // fix result popup box
    ptag = CB_PTAG(TR_CENTER, TR_CENTER);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultLevelup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultWindLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultThunderLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultWaterLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultFireLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultEarthLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultDoubleLvup, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pResultGetItem, ptag);
    
    // fix result window
    ptag = CB_PTAG(TR_LOW, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) this->m_pWinPanel, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_pLosePanel, ptag);
    for (i = 0; i < 4; i++) {
        set_uiwnd_ptag((struct UIWnd *) this->m_pResultWindow[i], ptag);
    }
    
    
    return true;
}


MAKE_PATCHSET(fixcombatui)
{
    if (sizeof(struct CCBUI) != 0xA48) fail("sizeof(struct CCBUI) mismatch.");
    
    cb_scalefactor = str2scalefactor(get_string_from_configfile("fixcombatui_scalefactor"));
    ccbui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixcombatui_scaletype"));

    // fix C2DSpark
    patch_c2dspark();
    INIT_ASMPATCH(fixattacksequen_c2dspark, 0x004DC4F0, 0x7, "\x6A\x24\x68\xB3\x02\x00\x00");
    INIT_ASMPATCH(fix_statuspanel_c2dspark, 0x004F85F5, 0x7, "\x68\x3A\x02\x00\x00\x6A\x19");
    INIT_ASMPATCH(fix_statuspanel_c2dspark, 0x004F862F, 0x7, "\x68\x3A\x02\x00\x00\x6A\x19");
    
    // hook CCBUI::Create
    INIT_WRAPPER_CALL(CCBUI_Create_wrapper, { 0x0051270B });
}
