#include "common.h"

#define SCENE_PTAG(scalefactor, srcrect, lr, tb) MAKE_PTAG((scalefactor), (srcrect), sceneui_dstrect_type, (lr), (tb))

static int sceneui_dstrect_type;
#define sceneui_dstrect (*get_ptag_frect(sceneui_dstrect_type))
#define sceneui_dstrect_scalefactor get_frect_min_scalefactor(&sceneui_dstrect, &game_frect_original)

// fix PlayerMgr: pushable notification, ShenYan numbers
static MAKE_THISCALL(void, HeadMsg_MsgDlg_Render_wrapper, struct UIStatic *this)
{
    set_uiwnd_ptag(pUIWND(this), MAKE_PTAG(SF_SCENETEXT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_MID, TR_SCALE_HIGH));
    push_ptag_state(pUIWND(this));
    UIWnd_vfptr_Render(pUIWND(this));
    pop_ptag_state(pUIWND(this));
}
static MAKE_THISCALL(void, HeadMsg_Render_wrapper, struct HeadMsg *this)
{
    // step2: fix lost blood message in scene
    int flag = this->m_bEnable && (this->m_nType == 0 || this->m_nType == 3);
    if (flag) fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, sceneicon_scalefactor);
    HeadMsg_Render(this);
    if (flag) fixui_popstate();
}
static void __cdecl PlayerMgr_DrawMsg_hookpart1()
{
    set_uiwnd_ptag(pUIWND(&g_msgbk), SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW));
    
    // step1: transform pushable notification
    push_ptag_state(pUIWND(&g_msgbk));
    
    PlayerMgr_DrawMsg(); // oldcode, it will call part2
    
    // step4: restore fixui state stack
    fixui_popstate();
}
static MAKE_THISCALL(void, PlayerMgr_DrawMsg_hookpart2, struct UIAnimateCtrl *this)
{
    // step3: change fixui state, for g_jumpflag and ShenYan
    pop_ptag_state(pUIWND(&g_msgbk));
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, sceneicon_scalefactor);
    
    UIAnimateCtrl_Render(this); // oldcode
}
static void fix_headmsg_offset()
{
    PATCH_FLOAT_MEMREF_EXPR(scale_gbxdiff(0.1f, sceneicon_scalefactor), { 0x00403985 });
}
static void fix_scene_lostblood()
{
    // override call to UIStatic::Render(m_pMsgDlg) in HeadMsg::Render()
    make_call(0x004037BF, HeadMsg_MsgDlg_Render_wrapper);
    
    // init wrapper to HeadMsg::Render()
    INIT_WRAPPER_CALL(HeadMsg_Render_wrapper, {
        0x0040C255,
        0x0051CD82,
    });
    
    // scale diff constant for showing lost blood
    add_postd3dcreate_hook(fix_headmsg_offset);
}
static void install_PlayerMgr_DrawMsg_hook()
{
    // fix scene lost blood information
    fix_scene_lostblood();
    
    // hookpart1 is a wrapper to PlayerMgr::DrawMsg()
    INIT_WRAPPER_CALL(PlayerMgr_DrawMsg_hookpart1, {
        0x00407BCD,
        0x00517F44,
    });
    
    // hookpart2 is a wrapper to a function call in middle of PlayerMgr::DrawMsg()
    INIT_WRAPPER_CALL(PlayerMgr_DrawMsg_hookpart2, { 0x0040C268 });
}



// fix UISceneMap::Render(): scene small map
static fPOINT scenemap_src_center, scenemap_dst_center;
static MAKE_ASMPATCH(UISceneMap_Render_hookpart1)
{
    // step1: transform small map rect
    fRECT src_frect, dst_frect;
    
    set_frect_rect(&src_frect, TOPTR(R_EBP - 0x14));
    fixui_adjust_fRECT(&dst_frect, &src_frect);
    floor_frect(&dst_frect, &dst_frect);

    frect2gbfrect(&src_frect, &src_frect);
    frect2gbfrect(&dst_frect, &dst_frect);
    
    set_fpoint(&scenemap_src_center, (src_frect.left + src_frect.right) / 2.0, (src_frect.top + src_frect.bottom) / 2.0);
    set_fpoint(&scenemap_dst_center, (dst_frect.left + dst_frect.right) / 2.0, (dst_frect.top + dst_frect.bottom) / 2.0);
    
    fixui_pushstate(&src_frect, &dst_frect, TR_SCALE_SIMPLE, TR_SCALE_SIMPLE, 1.0);
    fs->gb_align = 1;
    
    R_ESI += 0x0010AF24; // oldcode
}
static MAKE_ASMPATCH(UISceneMap_Render_hookpart2)
{
    // step2: pop previous state, set rect and push identity
    fixui_popstate();
    fixui_pushidentity();
    fs->no_align = 1;
    
    if (M_DWORD(R_EBX + 0x45C) != 2) { // oldcode
        LINK_JMP(0x0045930C);
    }
}
static MAKE_ASMPATCH(UISceneMap_Render_fixui_stack_balance)
{
    fixui_pushidentity();
    
    M_DWORD(R_EBX + 0x4B4) = R_ESI; // oldcode
}
static MAKE_THISCALL(void, UISceneMap_Render_hookpart3, struct UIWnd *this)
{
    // step3: restore fixui state
    fixui_popstate();
    UIWnd_Render_rewrited(this); // should call rewrited version
}

static MAKE_THISCALL(void, UISceneMap_Render_fixroleicon_gbMatrixStack_Translate_wrapper, struct gbMatrixStack *this, float x, float y, float z)
{
    gbMatrixStack_Translate(this, scenemap_dst_center.x, scenemap_dst_center.y, 0.0f);
    gbMatrixStack_Scale(this, sceneui_scalefactor, sceneui_scalefactor, 1.0f);
    gbMatrixStack_Translate(this, -scenemap_src_center.x, -scenemap_src_center.y, 0.0f);
    gbMatrixStack_Translate(this, x, y, z);
}

static MAKE_THISCALL(void, UISceneMap_Render_fixroleicon_gbMatrixStack_Rotate_wrapper, struct gbMatrixStack *this, float angle, struct gbVec3D *axis)
{
    gbMatrixStack_Scale(this, (4.0 / 3.0) / get_frect_aspect_ratio(&game_frect), 1.0, 1.0);
    gbMatrixStack_Rotate(this, angle, axis);
    gbMatrixStack_Scale(this, get_frect_aspect_ratio(&game_frect) * 0.75, 1.0, 1.0);
}

static void install_UISceneMap_Render_hook()
{
    // ensure pair fixui_pushstate() and fixui_popstate()
    INIT_ASMPATCH(UISceneMap_Render_fixui_stack_balance, 0x00459300, 6, "\x89\xB3\xB4\x04\x00\x00"); 

    // hookpart1 is an asmpatch
    INIT_ASMPATCH(UISceneMap_Render_hookpart1, 0x00458E62, 6, "\x81\xC6\x24\xAF\x10\x00");

    // hookpart2 is an asmpatch
    INIT_ASMPATCH(UISceneMap_Render_hookpart2, 0x00458E84, 6, "\x0F\x85\x82\x04\x00\x00");
    
    // hookpart3 is an wrapper to UIWnd::Render()
    INIT_WRAPPER_CALL(UISceneMap_Render_hookpart3, { 0x0045930E });
    
    // fix role icon size
    INIT_WRAPPER_CALL(UISceneMap_Render_fixroleicon_gbMatrixStack_Translate_wrapper, {
        0x00458FCC,
        0x004592BC,
    });
    INIT_WRAPPER_CALL(UISceneMap_Render_fixroleicon_gbMatrixStack_Rotate_wrapper, {
        0x004590C4,
        0x00459085,
    });
}



// fix UIEmote::Render() to fix emote
static MAKE_THISCALL(void, UIEmote_Render, struct UIEmote *this)
{
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_HIGH, sceneicon_scalefactor);
    UIWnd_Render_rewrited(pUIWND(this)); // should call rewrited version
    fixui_popstate();
}

// general sceneui fixes
static MAKE_THISCALL(void, UIGameFrm_Create_wrapper, struct UIGameFrm *this)
{
    UIGameFrm_Create(this);
    struct uiwnd_ptag ptag;
    
    ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    set_uiwnd_ptag(pUIWND(&this->m_ChatRest), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_FinishSave), ptag);
    
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_note), ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag(pUIWND(&this->m_seldlg), ptag);
    
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, TR_SCALE_HIGH, TR_SCALE_LOW);
    ptag.self_dstrect_use43 = 1;
    set_uiwnd_ptag(pUIWND(&this->m_cap), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_map), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_face), ptag);
}

static void pre_UIGameFrm(struct UIWnd *this)
{
    struct UIGameFrm *gamefrm = TOPTR(this);    
    struct uiwnd_ptag ptag;
    
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag(pUIWND(&gamefrm->m_selemote_frame), ptag);
    
    set_alt_father(FAPTR_UIGAMEFRM_SELEMOTE_FRAME, pUIWND(&gamefrm->m_selemote_frame));

    set_uiwnd_ptag(pUIWND(&gamefrm->m_selemote_BTN1), MAKE_ALT_FATHER_PTAG(FAPTR_UIGAMEFRM_SELEMOTE_FRAME));
    set_uiwnd_ptag(pUIWND(&gamefrm->m_selemote_BTN2), MAKE_ALT_FATHER_PTAG(FAPTR_UIGAMEFRM_SELEMOTE_FRAME));
}

static void post_UIGameFrm(struct UIWnd *this)
{
}

static MAKE_UIWND_RENDER_WRAPPER_CUSTOM(UIGameFrm_Render_wrapper, 0x00455E39, pre_UIGameFrm, post_UIGameFrm)
static MAKE_UIWND_UPDATE_WRAPPER_CUSTOM(UIGameFrm_Update_wrapper, 0x00455D6E, pre_UIGameFrm, post_UIGameFrm)

static void fix_gamescene()
{
    // hook UIGameFrm::Create()
    INIT_WRAPPER_CALL(UIGameFrm_Create_wrapper, {
        0x00406A73,
        0x00406AC2,
    });
    
    // hook UIGameFrm::Render() and UIGameFrm::Update()
    INIT_WRAPPER_VFPTR(UIGameFrm_Render_wrapper, 0x0055901C);
    INIT_WRAPPER_CALL(UIGameFrm_Render_wrapper, {
        0x004EDC64,
        0x00407C1B,
    });
    INIT_WRAPPER_VFPTR(UIGameFrm_Update_wrapper, 0x00559020);
    INIT_WRAPPER_CALL(UIGameFrm_Update_wrapper, {
        0x00407449,
    });
    
    // hook PlayerMgr::DrawMsg()
    install_PlayerMgr_DrawMsg_hook();
    
    // hook UISceneMap::Render()
    install_UISceneMap_Render_hook();
    
    // hook UIEmote::Render()
    INIT_WRAPPER_VFPTR(UIEmote_Render, 0x00558FD0);
    
    // fix ChatRest dialog position
    SIMPLE_PATCH(0x00450394, "\x05\x2C\x01\x00\x00", "\x05\x10\x01\x00\x00", 5);
}



// fix RoleDialog
#define ROLEDLG_FACESIZE 2.0 // ratio of text frame
#define ROLEDLG_FACEHEIGHTFACTOR (256.0 / 512.0) // real face height : face texture height
#define ROLEDLG_LEFTFACE_TEXTSHIFT 0.03
#define ROLEDLG_RIGHTFACE_TEXTSHIFT -0.03
static double dlg_minsize; // ratio of whole screen
static int dlg_margin; // text margin flag
static fRECT dlg_frect; // screen rect of role dialog
static fRECT dlg_real_frect; // real role dialog rect before scale
static fRECT dlg_old_frect; // original role dialog rect (effective)
static fRECT dlg_old_ref_frect; // original role dialog rect (reference)
static fRECT dlg_facearea_frect; // face area
static struct UIStaticVtbl *dlg_bk_oldvtbl = TOPTR(0x00558DB8); // original vftable for m_bk
static struct UIStaticVtbl *dlg_bk_newvtbl; // hooked vftable for m_bk

static MAKE_THISCALL(void, UIRoleDialog_SetFace_wrapper, struct UIRoleDialog *this, const char *path, int leftright)
{
    // call SetFace() with original rect
    set_rect_frect(&pUIWND(this)->m_rect, &dlg_old_ref_frect);
    push_drvinfo_setwh(GAME_WIDTH_ORG, GAME_HEIGHT_ORG);
    UIRoleDialog_SetFace(this, path, leftright);
    pop_drvinfo();
    set_rect_frect(&pUIWND(this)->m_rect, &dlg_real_frect);
    
    // get original textbox rect and arrow rect
    fRECT text_frect; // textbox
    fRECT ani_frect; // arrow
    set_frect_rect(&text_frect, &pUIWND(&this->m_static)->m_rect);
    set_frect_rect(&ani_frect, &pUIWND(&this->m_ani)->m_rect);

    // transform textarea rect and arrow rect
    double text_ani_distance = ani_frect.left - text_frect.right;
    double text_bk_distance = text_frect.top - dlg_old_frect.top;
    fRECT textarea_frect; // area avaliable for displaying text
    transform_frect(&ani_frect, &ani_frect, &dlg_old_frect, &dlg_real_frect, TR_HIGH, TR_HIGH, 1.0);
    transform_frect(&textarea_frect, &text_frect, &dlg_old_frect, &dlg_real_frect, TR_LOW, TR_HIGH, 1.0);
    if (path) { // have face image
        // adjust face
        fRECT face_frect, face_old_frect;
        set_frect_rect(&face_old_frect, &pUIWND(&this->m_face)->m_rect);
        transform_frect(&face_frect, &face_old_frect, &game_frect_original_lt, &dlg_facearea_frect, (leftright ? TR_HIGH : TR_LOW), TR_HIGH, 1.0);
        scenedlgface_scalefactor = fmin(ROLEDLG_FACESIZE * get_frect_height(&dlg_frect) / (get_frect_height(&face_frect) * ROLEDLG_FACEHEIGHTFACTOR), sceneui_dstrect_scalefactor);
        set_uiwnd_ptag(pUIWND(&this->m_face), MAKE_PTAG(SF_SCENEDLGFACE, PTR_GAMERECT, PTR_GAMERECT, (leftright ? TR_SCALE_HIGH : TR_SCALE_LOW), TR_SCALE_HIGH));
        set_rect_frect(&pUIWND(&this->m_face)->m_rect, &face_frect);
        
        // adjust text
        double face_shift = get_frect_width(&face_old_frect) * (1.0 - scenedlgface_scalefactor / scenetext_scalefactor);
        if (leftright) { // right
            translate_frect_rel(&ani_frect, &ani_frect, face_shift, 0.0);
        } else { // left
            textarea_frect.left -= face_shift;
        }
    }
    textarea_frect.top = dlg_real_frect.top + text_bk_distance;
    textarea_frect.right = ani_frect.left - text_ani_distance;
    
    // calc textbox location
    fRECT new_text_frect;
    if (dlg_margin) {
        fRECT ideal_textarea_frect;
        set_frect_ltrb(&ideal_textarea_frect, dlg_real_frect.left, textarea_frect.top, dlg_real_frect.right, textarea_frect.bottom);
        transform_frect(&new_text_frect, &text_frect, &ideal_textarea_frect, &ideal_textarea_frect, TR_CENTER, TR_CENTER, 1.0);
        if (path) { // have face image
            if (leftright) { // right
                translate_frect_rel(&new_text_frect, &new_text_frect, ROLEDLG_RIGHTFACE_TEXTSHIFT * get_frect_width(&ideal_textarea_frect), 0.0);
            } else { // left
                translate_frect_rel(&new_text_frect, &new_text_frect, ROLEDLG_LEFTFACE_TEXTSHIFT * get_frect_width(&ideal_textarea_frect), 0.0);
            }
        }
        if (new_text_frect.left < textarea_frect.left) {
            translate_frect_rel(&new_text_frect, &new_text_frect, textarea_frect.left - new_text_frect.left, 0.0);
        } else if (new_text_frect.right > textarea_frect.right) {
            translate_frect_rel(&new_text_frect, &new_text_frect, textarea_frect.right - new_text_frect.right, 0.0);
        }
    } else {
        new_text_frect = textarea_frect;
    }
    
    // set rect for textbox and arrow
    set_rect_frect(&pUIWND(&this->m_static)->m_rect, &new_text_frect);
    set_rect_frect(&pUIWND(&this->m_ani)->m_rect, &ani_frect);
}
static MAKE_THISCALL(void, UIRoleDialog_Create_wrapper, struct UIRoleDialog *this, int id, RECT *rect, struct UIWnd *pfather, const char *bkfile)
{
    push_drvinfo_setwh(GAME_WIDTH_ORG, GAME_HEIGHT_ORG);
    UIRoleDialog_Create(this, id, rect, pfather, bkfile);
    pop_drvinfo();
    
    // hook m_bk->vfptr
    this->m_bk.vfptr = (struct UIWndVtbl *) dlg_bk_newvtbl;
    
    // calc role dialog rect
    set_frect_rect(&dlg_old_ref_frect, &pUIWND(this)->m_rect);
    set_frect_rect(&dlg_old_frect, &this->m_bkRc);
    transform_frect(&dlg_frect, &dlg_old_frect, &game_frect_original_lt, &sceneui_dstrect, TR_CENTER, TR_HIGH, sceneui_dstrect_scalefactor);
    dlg_frect.top = fmin(dlg_frect.bottom - get_frect_height(&dlg_old_frect) * scenetext_scalefactor, sceneui_dstrect.top + get_frect_height(&sceneui_dstrect) * (1.0 - dlg_minsize));
    
    transform_frect(&dlg_real_frect, &dlg_frect, &dlg_frect, &dlg_frect, TR_LOW, TR_LOW, 1.0 / scenetext_scalefactor);
    set_rect_frect(&pUIWND(this)->m_rect, &dlg_real_frect);
    set_uiwnd_ptag(pUIWND(this), MAKE_PTAG(SF_SCENETEXT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_LOW, TR_SCALE_LOW));
    
    // calc face area rect (not face rect itself)
    dlg_facearea_frect = dlg_frect;
    dlg_facearea_frect.bottom = sceneui_dstrect.bottom;
    
    // update face and textbox positions
    THISCALL_WRAPPER(UIRoleDialog_SetFace_wrapper, this, NULL, 0);
}
static MAKE_THISCALL(void, bk_Render, struct UIWnd *this)
{
    fixui_pushstate(&dlg_old_frect, &dlg_frect, TR_SCALE_SIMPLE, TR_SCALE_SIMPLE, 1.0);
    THISCALL_WRAPPER(dlg_bk_oldvtbl->Render, this);
    fixui_popstate();
}
static MAKE_UIWND_RENDER_WRAPPER(UIRoleDialog_Render_wrapper, 0x0045742A)
static MAKE_UIWND_UPDATE_WRAPPER(UIRoleDialog_Update_wrapper, 0x004576CC)
static void fix_RoleDialog()
{
    dlg_bk_newvtbl = dup_vftable(dlg_bk_oldvtbl, sizeof(struct UIStaticVtbl));
    dlg_bk_newvtbl->Render = bk_Render;
    
    // manually add wrapper to UIRoleDialog::Render/Update()
    INIT_WRAPPER_VFPTR(UIRoleDialog_Render_wrapper, 0x005590D4);
    INIT_WRAPPER_VFPTR(UIRoleDialog_Update_wrapper, 0x005590D8);
    
    // hook UIRoleDialog::Create
    INIT_WRAPPER_CALL(UIRoleDialog_Create_wrapper, {
        0x004558D2,
        //0x005207A6, // no need to hook comp-donate releated calls
    });
    
    // hook UIRoleDialog::SetFace
    INIT_WRAPPER_CALL(UIRoleDialog_SetFace_wrapper, {
        0x0043F25B,
        0x00455946,
        0x0045596F,
        0x00455998,
        0x004559C1,
        0x004559EA,
        0x00455A13,
        0x00455A3C,
        0x004576B2,
        0x004578A3,
        //0x00520B8C, // no need to hook comp-donate releated calls
        //0x00520D91,
        //0x0052103B,
        //0x005211CB,
        //0x00521293,
    });
}






// fix UIGameOver
static MAKE_THISCALL(void, UIGameOver_Create_wrapper, struct UIGameOver *this)
{
    UIGameOver_Create(this);
    
    set_uiwnd_ptag(pUIWND(&this->m_Blood), MAKE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, PTR_GAMERECT, TR_SCALE_MID, TR_SCALE_MID));
}
static void fix_UIGameOver()
{
    INIT_WRAPPER_CALL(UIGameOver_Create_wrapper, { 0x004561DC });
}






// fix CG_UI

static void pre_CG_UI(struct UIWnd *this)
{
    struct CG_UI *ui = TOPTR(this);
    struct uiwnd_ptag ptag;
    int i;
    
    // tools
    set_alt_father(FAPTR_CGUI_TOOLS, pUIWND(&ui->m_pTools));
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT_ORIGINAL, TR_HIGH, TR_CENTER);
    set_uiwnd_ptag(pUIWND(&ui->m_pTools), ptag);

    set_uiwnd_ptag(pUIWND(&ui->m_pTime), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pMoney), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    for (i = 0; i < 9; i++) {
        set_uiwnd_ptag(pUIWND(&ui->m_pItems[i]), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    }
    set_uiwnd_ptag(pUIWND(&ui->m_pName), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pPrice), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pHurt), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pInfo), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pBuy), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_pRun), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    set_uiwnd_ptag(pUIWND(&ui->m_Exit), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));

    
    // prop
    set_alt_father(FAPTR_CGUI_PROP, pUIWND(&ui->m_pProp));
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag(pUIWND(&ui->m_pProp), ptag);

    for (i = 0; i < 5; i++) {
        set_uiwnd_ptag(pUIWND(&ui->m_pProps[i]), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_PROP));
        set_uiwnd_ptag(pUIWND(&ui->m_pPropsNum[i]), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_PROP));
    }
    
    
    // HP
    set_alt_father(FAPTR_CGUI_HP, pUIWND(&ui->m_pHP));
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_LOW);
    set_uiwnd_ptag(pUIWND(&ui->m_pHP), ptag);
    
    set_uiwnd_ptag(pUIWND(&ui->m_pBossHP), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_HP));
    set_uiwnd_ptag(pUIWND(&ui->m_pRoleHP), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_HP));
    set_uiwnd_ptag(pUIWND(&ui->m_pBossFace), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_HP));
    set_uiwnd_ptag(pUIWND(&ui->m_pRoleFace), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_HP));


    // TimeBackground
    set_alt_father(FAPTR_CGUI_TIMEBACKGROUND, pUIWND(&ui->m_pTimeBackground));
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(&ui->m_pTimeBackground), ptag);
    
    set_uiwnd_ptag(pUIWND(&ui->m_pTimeHunDigit), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TIMEBACKGROUND));
    set_uiwnd_ptag(pUIWND(&ui->m_pTimeTenDigit), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TIMEBACKGROUND));
    set_uiwnd_ptag(pUIWND(&ui->m_pTimeSinDigit), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TIMEBACKGROUND));


    // scene map
    if (ui->m_pTools.m_bvisible) {
        set_uiwnd_ptag(pUIWND(&ui->m_SceneMap), MAKE_ALT_FATHER_PTAG(FAPTR_CGUI_TOOLS));
    } else {
        set_uiwnd_ptag(pUIWND(&ui->m_SceneMap), SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_HIGH));
    }
    
    // result    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT_ORIGINAL, TR_CENTER, TR_SCALE_LOW);
    ptag.self_dstrect_use43 = 1;
    set_uiwnd_ptag(pUIWND(&ui->m_pResult), ptag);
    
    // dialog
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT_ORIGINAL, TR_CENTER, TR_CENTER);
    ptag.self_dstrect_use43 = 1;
    
    RECT yesno_dlgrc;
    set_rect_ltwh(&yesno_dlgrc, 250 + (PAL3_s_drvinfo.width - 800) / 2, 200 + (PAL3_s_drvinfo.height - 600) / 2, 256, 128);
    set_uiwnd_ptag(pUIWND(&ui->m_pWinAnswer), ptag); pUIWND(&ui->m_pWinAnswer)->m_rect = yesno_dlgrc;
    set_uiwnd_ptag(pUIWND(&ui->m_pLoseAnswer), ptag); pUIWND(&ui->m_pLoseAnswer)->m_rect = yesno_dlgrc;
    set_uiwnd_ptag(pUIWND(&ui->m_pExitDlg), ptag); pUIWND(&ui->m_pExitDlg)->m_rect = yesno_dlgrc;
    
    set_uiwnd_ptag(pUIWND(&ui->m_pNoMoneyMsg), ptag); pUIWND(&ui->m_pNoMoneyMsg)->m_rect = pUIWND(&ui->m_pNoMoneyMsg.m_bk)->m_rect;
    set_uiwnd_ptag(pUIWND(&ui->m_pSurpassMsg), ptag); pUIWND(&ui->m_pSurpassMsg)->m_rect = pUIWND(&ui->m_pSurpassMsg.m_bk)->m_rect;
    set_uiwnd_ptag(pUIWND(&ui->m_pAccountMsg), ptag); pUIWND(&ui->m_pAccountMsg)->m_rect = pUIWND(&ui->m_pAccountMsg.m_bk)->m_rect;
    set_uiwnd_ptag(pUIWND(&ui->m_pOverGameMsg), ptag); pUIWND(&ui->m_pOverGameMsg)->m_rect = pUIWND(&ui->m_pOverGameMsg.m_bk)->m_rect;
    
    
    // update InvalidRect
    push_ptag_state(pUIWND(&ui->m_pTools));
    fixui_adjust_RECT(&ui->m_InvalidRc0, &ui->m_pTools.m_rect);
    pop_ptag_state(pUIWND(&ui->m_pTools));
}
static void post_CG_UI(struct UIWnd *this)
{
}

static MAKE_UIWND_RENDER_WRAPPER_CUSTOM(CG_UI_Render_wrapper, 0x0051CE29, pre_CG_UI, post_CG_UI)
static MAKE_UIWND_UPDATE_WRAPPER_CUSTOM(CG_UI_Update_wrapper, 0x0051CEBF, pre_CG_UI, post_CG_UI)

static void fix_CG_UI()
{
    INIT_WRAPPER_VFPTR(CG_UI_Render_wrapper, 0x0055EEB0);
    INIT_WRAPPER_VFPTR(CG_UI_Update_wrapper, 0x0055EEB4);
    
    SIMPLE_PATCH(0x0051ACFB, "\x0F\x85\x1E\x01\x00\x00", "\xE9\x1F\x01\x00\x00\x90", 6);
}







MAKE_PATCHSET(fixsceneui)
{
    sceneui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixsceneui_scaletype"));
    sceneui_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_uiscalefactor"));
    sceneicon_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_iconscalefactor"));
    scenetext_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_textscalefactor"));
    
    dlg_minsize = str2double(get_string_from_configfile("fixsceneui_dlgminsize"));
    dlg_margin = get_int_from_configfile("fixsceneui_dlgmargin");

    // general fixes
    fix_gamescene();
    
    // fix RoleDialog
    fix_RoleDialog();
    
    // fix UIGameOver
    fix_UIGameOver();
    
    // fix CG_UI
    fix_CG_UI();
}
