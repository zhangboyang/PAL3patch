#include "common.h"

#define CB_POPSTATE fixui_popstate

#define CB_PUSHSTATE(lr, tb) fixui_pushstate(     &game_frect , get_ptag_frect(ccbui_dstrect_type), (lr), (tb), cb_scalefactor)
#define CB_PTAG(lr, tb)      MAKE_PTAG(SF_COMBAT, PTR_GAMERECT, ccbui_dstrect_type                , (lr), (tb))

static int ccbui_dstrect_type;

// patch C2DSpark, only size is changed, position is not changed
// FIXME: there may be better ways to do the same job using transform framework
static MAKE_THISCALL(void, C2DSpark_Render_wrapper, struct C2DSpark *this)
{
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_MID, cb_scalefactor);
    C2DSpark_Render(this);
    fixui_popstate();
}
static MAKE_THISCALL(bool, C2DSpark_CreateSingle_wrapper, struct C2DSpark *this, struct C2DSpark_tagSpark *pSpark)
{
    struct C2DSpark_tagSpark tmp = *pSpark;
    tmp.fVx *= cb_scalefactor; tmp.fAx *= cb_scalefactor;
    tmp.fVy *= cb_scalefactor; tmp.fAy *= cb_scalefactor;
    return C2DSpark_CreateSingle(this, &tmp);
}
static MAKE_THISCALL(void, C2DSpark_CreateStars_wrapper, struct C2DSpark *this, int x, int y, int nWidth, float fStarSize)
{
    C2DSpark_CreateStars(this, x, y, nWidth * cb_scalefactor, fStarSize);
}
static MAKE_THISCALL(void, C2DSpark_CreateMore_wrapper, struct C2DSpark *this, int x, int y, struct gbColorQuad color)
{
    POINT pt = {x, y};
    CB_PUSHSTATE(TR_HIGH, TR_LOW);
    fixui_adjust_POINT(&pt, &pt);
    CB_POPSTATE();
    C2DSpark_CreateMore(this, pt.x, pt.y, color);
}
static void patch_c2dspark()
{
    // fix random range
    INIT_WRAPPER_CALL(C2DSpark_CreateStars_wrapper, {
        0x004E96EB,
        0x004E971F,
    });
    
    // fix initial velocity and acceleration
    INIT_WRAPPER_CALL(C2DSpark_CreateSingle_wrapper, {
        0x004C9FEB,
        0x004C9E60,
        0x004CA2EF,
    });
    
    // fix spark size
    INIT_WRAPPER_CALL(C2DSpark_Render_wrapper, { 0x00506AA3 });
    
    // fix spart position
    INIT_WRAPPER_CALL(C2DSpark_CreateMore_wrapper, {
        0x004CFCA3,
        //0x0050EC43, // should not hook CCBUI::Update because imbibe nimbus spark use real coord
    });
}




// fix CCBDisplayChain
static int CCBDisplayChain_Render_popflag;
static MAKE_ASMPATCH(CCBDisplayChain_Render_PreEachItem)
{
    struct tagShowItem *cur = TOPTR(R_ESI - 0x114);
    switch (cur->eKind) {
        case CBSIK_Txt:
            fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_HIGH, cb_scalefactor);
            CCBDisplayChain_Render_popflag = 1;
            break;
        case CBSIK_RoleState:
            if (gby2y(cur->fY) < 600 / 2) {
                // top half, result window
                CB_PUSHSTATE(TR_LOW, TR_LOW);
            } else {
                // bottom half, status panel
                CB_PUSHSTATE(TR_LOW, TR_HIGH);
            }
            CCBDisplayChain_Render_popflag = 1;
            break;
        default: CCBDisplayChain_Render_popflag = 0; break;
    }
    R_EAX = M_DWORD(R_ESI - 0x114); // oldcode
}
static MAKE_ASMPATCH(CCBDisplayChain_Render_PostEachItem)
{
    if (CCBDisplayChain_Render_popflag) {
        fixui_popstate();
        CCBDisplayChain_Render_popflag = 0;
    }
    R_ESI += 0x118; // oldcode
}



// fix others
static MAKE_THISCALL(bool, CCBUI_Create_wrapper, struct CCBUI *this)
{
    if (!CCBUI_Create(this)) return false;
    
    int i, j;
    struct uiwnd_ptag ptag;
    
    // fix rolestate panel
    ptag = CB_PTAG(TR_LOW, TR_HIGH);
    for (i = 0; i < 4; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pRoleStateHP[i]), ptag);
        set_uiwnd_ptag(pUIWND(this->m_pRoleStateGP[i]), ptag);
        set_uiwnd_ptag(pUIWND(this->m_pRoleStateMP[i]), ptag);
        set_uiwnd_ptag(pUIWND(this->m_pRoleStatePanel[i]), ptag);
    }
    for (i = 0; i < 5; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pRoleStateFace[i]), ptag);
        set_uiwnd_ptag(pUIWND(this->m_pRoleStateFaceName[i]), ptag);
    }
    
    // fix rolestate icons
    for (i = 0; i < 11; i++) {
        if (i < 4) {
            ptag = CB_PTAG(TR_LOW, TR_HIGH);
        } else {
            ptag = MAKE_PTAG(SF_COMBAT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_LOW, TR_SCALE_LOW);
        }
        for (j = 0; j < 19; j++) {
            set_uiwnd_ptag(pUIWND(this->m_pRoleSpecState[j][i]), ptag);
        }
    }
    
    // fix trickname
    ptag = CB_PTAG(TR_CENTER, TR_LOW);
    set_uiwnd_ptag(pUIWND(this->m_pTrickName), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pTrickName2), ptag);
    
    // fix attacksequen
    ptag = CB_PTAG(TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(this->m_pAttackSequenBack), ptag);
    for (i = 0; i < 11; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pAttackSequenFace[i]), ptag);
    }

    // fix five nimbus
    ptag = CB_PTAG(TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(this->pPanel), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pFiveNimbusWindow), ptag);
    for (i = 0; i < 6; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pAttackSequenFace[i]), ptag);
    }
    
    // fix result popup box
    ptag = CB_PTAG(TR_CENTER, TR_CENTER);
    set_uiwnd_ptag(pUIWND(this->m_pResultLevelup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultWindLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultThunderLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultWaterLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultFireLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultEarthLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultDoubleLvup), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pResultGetItem), ptag);
    
    // fix result window
    ptag = CB_PTAG(TR_LOW, TR_LOW);
    set_uiwnd_ptag(pUIWND(this->m_pWinPanel), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pLosePanel), ptag);
    for (i = 0; i < 4; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pResultWindow[i]), ptag);
    }
    
    // fix other windows
    ptag = CB_PTAG(TR_LOW, TR_HIGH);
    set_uiwnd_ptag(pUIWND(this->m_pMain), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pItemWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pParabolaWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pMagicWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pSkillWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pAIWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pLineupWindow), ptag);
    set_uiwnd_ptag(pUIWND(this->m_pProtectWindow), ptag);
    
    // fix lineup faces positions
    ptag = MAKE_PTAG(SF_COMBAT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_LOW, TR_SCALE_LOW);
    for (i = 0; i < 5; i++) {
        set_uiwnd_ptag(pUIWND(this->m_pLineupWindow->m_pFace[i]), ptag);
    }
    int *lineup_head_left = TOPTR(0x005EC16C);
    int *lineup_head_top = TOPTR(0x005EC188);
    CB_PUSHSTATE(TR_LOW, TR_HIGH);
    for (i = 1; i <= 6; i++) {
        int xoffset = M_DWORD(0x020B58A0), yoffset = M_DWORD(0x020B589C);
        POINT pt = { .x = lineup_head_left[i] + xoffset, .y = lineup_head_top[i] + yoffset };
        fixui_adjust_POINT(&pt, &pt);
        lineup_head_left[i] = pt.x - xoffset;
        lineup_head_top[i] = pt.y - yoffset;
    }
    CB_POPSTATE();
    
    // fix combat dialog
    ptag = MAKE_PTAG(SF_COMBAT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_MID, TR_SCALE_HIGH);
    set_uiwnd_ptag(pUIWND(this->m_pDialogBack), ptag);
    
    return true;
}


static MAKE_THISCALL(void, CCBUI_Render, struct CCBUI *this)
{
    int i, j, k;
    
    // recalculate spec state icon positions for monsters
    int icon_seq[]   = {  0,  1,  2,  3,  6,  7,  4,  5,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    int icon_width[] = { 24, 24, 24, 24, 24, 24, 24, 24, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20}; 
    for (i = 5; i < 11; i++) {
        int left = 0, top = 0;
        int firstflag = 1;
        for (k = 0; k < 19; k++) {
            j = icon_seq[k];
            struct UIStatic *pWnd = this->m_pRoleSpecState[j][i];
            if (pWnd->m_bcreateok && pWnd->m_bvisible) {
                if (firstflag) {
                    left = pWnd->m_rect.left;
                    top = pWnd->m_rect.top;
                    firstflag = 0;
                }
                UIWnd_MoveWindow(pUIWND(pWnd), left, top);
                left += ceil(icon_width[j] * cb_scalefactor - eps);
            }
        }
    }
    
    // call baseclass's Render()
    UIFrameWnd_Render((struct UIFrameWnd *) this);
    
    // render imbible nimbus
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_MID, cb_scalefactor);
    for (i = 0; i < this->mImbibeNimbusNum; i++) {
        struct tagImbibeNimbus *cur = &this->m_pImbibeNimbus[i];
        if (cur->bAlive) {
            UIWnd_vfptr_Render(pUIWND(cur->mNimbus));
        }
    }
    fixui_popstate();
}


static MAKE_THISCALL(RECT *, CCBUI_GetNimbusArea_wrapper, struct CCBUI *this, RECT *rc, enum ECBFiveNimbus nimbustype)
{
    CCBUI_GetNimbusArea(this, rc, nimbustype);
    CB_PUSHSTATE(TR_HIGH, TR_LOW);
    fixui_adjust_RECT(rc, rc);
    CB_POPSTATE();
    return rc;
}


// rewrite CCBLineupWindow::Render
static MAKE_THISCALL(void, CCBLineupWindow_Render, struct CCBLineupWindow *this)
{
    UIFrameWnd_Render((struct UIFrameWnd *) this);
    struct UIWnd *pSelectedFace = pUIWND(this->m_pFace[this->m_nSelected]);
    push_ptag_state(pSelectedFace);
    UIWnd_vfptr_Render(pSelectedFace);
    pop_ptag_state(pSelectedFace);
}

// rewrite CCBLineupWindow::IsPtOnFace
static MAKE_THISCALL(bool, CCBLineupWindow_IsPtOnFace, struct CCBLineupWindow *this, int nFaceIndex, POINT pt)
{
    struct UIWnd *pFace = pUIWND(this->m_pFace[nFaceIndex]);
    RECT rc = pFace->m_rect;
    push_ptag_state(pFace);
    fixui_adjust_RECT(&rc, &rc);
    pop_ptag_state(pFace);
    return PtInRect(&rc, pt);
}

static void setcursorpos_ccbcontrol_hookfunc(void *arg)
{
    POINT *mousept = arg;
    // SetCursorPos is called only in CCBControl::Control
    // so we just translate the cursor position without any judgements
    CB_PUSHSTATE(TR_LOW, TR_HIGH);
    fixui_adjust_POINT(mousept, mousept);
    CB_POPSTATE();
}

static MAKE_ASMPATCH(role_hp_string_position_limiter)
{
    int lr = M_DWORD(R_EBP - 0x8);
    int tb = M_DWORD(R_EBP - 0x4);
    char *hp_string = TOPTR(0x020B3BD0);
    
    int cw = 10 * cb_scalefactor; // calc char width and height
    int ch = 20 * cb_scalefactor;
    
    int sw = cw * strlen(hp_string); // calc string width
    
    lr = imax(imin(lr, game_width - sw), 0);
    tb = imax(imin(tb, game_height), ch);
    
    M_DWORD(R_EBP - 0x8) = lr;
    M_DWORD(R_EBP - 0x4) = tb;
}

MAKE_PATCHSET(fixcombatui)
{
    cb_scalefactor = str2scalefactor(get_string_from_configfile("fixcombatui_scalefactor"));
    ccbui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixcombatui_scaletype"));

    // fix C2DSpark
    patch_c2dspark();

    // fix CCBDisplayChain::Render
    INIT_ASMPATCH(CCBDisplayChain_Render_PreEachItem, 0x004D30E3, 0x6, "\x8B\x86\xEC\xFE\xFF\xFF");
    INIT_ASMPATCH(CCBDisplayChain_Render_PostEachItem, 0x004D3302, 0x6, "\x81\xC6\x18\x01\x00\x00");

    // hook CCBUI::Create
    INIT_WRAPPER_CALL(CCBUI_Create_wrapper, { 0x0050A32D });

    // replace CCBUI::Render
    INIT_WRAPPER_VFPTR(CCBUI_Render, 0x0055EB78);
    
    // hook CCBUI::GetNimbusArea
    INIT_WRAPPER_CALL(CCBUI_GetNimbusArea_wrapper, {
        0x0050E5D3,
        0x0050EB7C,
    });

    // patch 1024x768 check
    SIMPLE_PATCH_NOP(0x0050D1B5, "\x7D\x03", 2);

    
    // hook SetCursorPos
    add_setcursorpos_hook(setcursorpos_ccbcontrol_hookfunc);

    // patch CCBLineupWindow
    make_jmp(0x0051263C, CCBLineupWindow_Render);
    make_jmp(0x0051265E, CCBLineupWindow_IsPtOnFace);

    // patch role HP string position limiter
    INIT_ASMPATCH(role_hp_string_position_limiter, 0x004FD0E5, 0x2C, "\x7D\x06\x83\x65\xF8\x00\xEB\x0F\xA1\x90\x17\xC0\x00\x2B\xC7\x39\x45\xF8\x7E\x03\x89\x45\xF8\x6A\x14\x58\x39\x45\xFC\x7C\x0A\xA1\x94\x17\xC0\x00\x39\x45\xFC\x7E\x03\x89\x45\xFC");
}
