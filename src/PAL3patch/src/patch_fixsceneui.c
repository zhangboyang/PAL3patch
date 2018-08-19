#include "common.h"

#define SCENE_PTAG(scalefactor, srcrect, lr, tb) MAKE_PTAG((scalefactor), (srcrect), sceneui_dstrect_type, (lr), (tb))

static int sceneui_dstrect_type;
#define sceneui_dstrect (*get_ptag_frect(sceneui_dstrect_type))
#define sceneui_dstrect_scalefactor get_frect_min_scalefactor(&sceneui_dstrect, &game_frect_original)

// fix PlayerMgr: pushable notification, ShenYan numbers
static MAKE_THISCALL(void, HeadMsg_Render_wrapper, struct HeadMsg *this)
{
    // step2: fix lost blood message in scene
    int flag = this->m_bEnable && !this->m_nType;
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
    PATCH_FLOAT_MEMREF_EXPR(scale_gbxdiff(0.1f, sceneicon_scalefactor), { 0x00402F76 });
}
static void fix_scene_lostblood()
{
    // init wrapper to HeadMsg::Render()
    INIT_WRAPPER_CALL(HeadMsg_Render_wrapper, { 0x0040C7F4 });
    
    // scale diff constant for showing lost blood
    add_postd3dcreate_hook(fix_headmsg_offset);
}
static void install_PlayerMgr_DrawMsg_hook()
{
    // fix scene lost blood information
    fix_scene_lostblood();
    
    // hookpart1 is a wrapper to PlayerMgr::DrawMsg()
    INIT_WRAPPER_CALL(PlayerMgr_DrawMsg_hookpart1, { 0x00405FE0 });
    
    // hookpart2 is a wrapper to a function call in middle of PlayerMgr::DrawMsg()
    INIT_WRAPPER_CALL(PlayerMgr_DrawMsg_hookpart2, { 0x0040C808 });
}



// fix UISceneMap::Render(): scene small map
static MAKE_ASMPATCH(UISceneMap_Render_hookpart1)
{
    RECT *rc = TOPTR(R_ESP + 0x10);
    struct UISceneMap *this = TOPTR(R_EDI);
    
    // oldcode
    *rc = this->m_mapbk.m_currect;
    rc->left += 5;
    rc->top += 10;
    rc->right -= 5;
    rc->bottom -= 5;
    
    // step1: transform rect and push identity
    fixui_adjust_RECT(rc, rc);
    fixui_pushidentity();
    fs->gb_align = 1;
}
static MAKE_THISCALL(void, UISceneMap_Render_hookpart2, struct UIWnd *this)
{
    // step2: restore fixui state
    fixui_popstate();
    UIWnd_Render_rewrited(this); // should call rewrited version
}
static MAKE_ASMPATCH(UISceneMap_Render_fixroleicon_part1)
{
    R_ECX = M_DWORD(R_EBX + 0x21F0); // oldcode
    
    struct gbMatrixStack *pview = TOPTR(R_ESI);
    gbMatrixStack_Scale(pview, sceneui_scalefactor, sceneui_scalefactor, 1.0);
    gbMatrixStack_Scale(pview, (4.0 / 3.0) / get_frect_aspect_ratio(&game_frect), 1.0, 1.0);
}
static MAKE_ASMPATCH(UISceneMap_Render_fixroleicon_part2)
{
    R_ECX = R_EDI + 0x3AC; // oldcode
    
    struct gbMatrixStack *pview = TOPTR(R_ESI);
    gbMatrixStack_Scale(pview, get_frect_aspect_ratio(&game_frect) * 0.75, 1.0, 1.0);
}
static void install_UISceneMap_Render_hook()
{
    // small asm trick to ensure pair fixui_pushstate() and fixui_popstate()
    SIMPLE_PATCH(0x00452B36, "\x0F\x85\xC8\x01\x00\x00", "\x0F\x85\xAD\x01\x00\x00", 6);

    // hookpart1 is an asmpatch
    INIT_ASMPATCH(UISceneMap_Render_hookpart1, 0x00452AFB, 7, "\x83\xEE\x05\x89\x74\x24\x1C");
    
    // hookpart2 is an wrapper to UIWnd::Render()
    INIT_WRAPPER_CALL(UISceneMap_Render_hookpart2, { 0x00452CEB });
    
    // fix role icon size
    INIT_ASMPATCH(UISceneMap_Render_fixroleicon_part1, 0x00452C96, 6, "\x8B\x8B\xF0\x21\x00\x00");
    INIT_ASMPATCH(UISceneMap_Render_fixroleicon_part2, 0x00452CAC, 6, "\x8D\x8F\xAC\x03\x00\x00");
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
    set_uiwnd_ptag(pUIWND(this->m_chosecompose), ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_note), ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag(pUIWND(&this->m_seldlg), ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, TR_SCALE_HIGH, TR_SCALE_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_cap), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_map), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_face), ptag);
    

}

static MAKE_UIWND_RENDER_WRAPPER(ChoseCompose_Render_wrapper, 0x00524970)
static MAKE_UIWND_UPDATE_WRAPPER(ChoseCompose_Update_wrapper, 0x0047E3E0)
static void fix_gamescene()
{
    // hook UIGameFrm::Create()
    INIT_WRAPPER_CALL(UIGameFrm_Create_wrapper, {
        0x00404B22,
        0x00404B7F,
    });
    
    // hook PlayerMgr::DrawMsg()
    install_PlayerMgr_DrawMsg_hook();
    
    // hook UISceneMap::Render()
    install_UISceneMap_Render_hook();
    
    // hook UIEmote::Render()
    INIT_WRAPPER_VFPTR(UIEmote_Render, 0x0056B054);
    
    // fix ChatRest dialog position
    SIMPLE_PATCH(0x0044AF8E, "\xC7\x44\x24\x10\x2C\x01\x00\x00", "\xC7\x44\x24\x10\xFA\x00\x00\x00", 8);
    
    // manually add wrapper to ChoseCompose::Render/Update()
    INIT_WRAPPER_VFPTR(ChoseCompose_Render_wrapper, 0x0056B8A8);
    INIT_WRAPPER_VFPTR(ChoseCompose_Update_wrapper, 0x0056B8AC);
}


// fix RoleDialog
#define ROLEDLG_FACESIZE 2.0 // ratio of text frame
#define ROLEDLG_FACEHEIGHTFACTOR (256.0 / 512.0) // real face height : face texture height
static double dlg_minsize; // ratio of whole screen
static int dlg_margin; // text margin flag
static fRECT dlg_frect; // screen rect of role dialog
static fRECT dlg_real_frect; // real role dialog rect before scale
static fRECT dlg_old_frect; // original role dialog rect
static fRECT dlg_facearea_frect; // face area

static MAKE_THISCALL(void, UIRoleDialog_SetFace_wrapper, struct UIRoleDialog *this, const char *path, int leftright)
{
    // call SetFace() with original rect
    set_rect_frect(&pUIWND(this)->m_rect, &dlg_old_frect);
    UIRoleDialog_SetFace(this, path, leftright);
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
        transform_frect(&face_frect, &face_old_frect, &game_frect, &dlg_facearea_frect, (leftright ? TR_HIGH : TR_LOW), TR_HIGH, 1.0);
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
    UIRoleDialog_Create(this, id, rect, pfather, bkfile);
    
    // calc role dialog rect
    set_frect_rect(&dlg_old_frect, &pUIWND(this)->m_rect);
    transform_frect(&dlg_frect, &dlg_old_frect, &game_frect_original, &sceneui_dstrect, TR_CENTER, TR_HIGH, sceneui_dstrect_scalefactor);
    dlg_frect.top = fmin(dlg_frect.bottom - get_frect_height(&dlg_old_frect) * scenetext_scalefactor, sceneui_dstrect.top + get_frect_height(&sceneui_dstrect) * (1.0 - dlg_minsize));
    
    transform_frect(&dlg_real_frect, &dlg_frect, &dlg_frect, &dlg_frect, TR_LOW, TR_LOW, 1.0 / scenetext_scalefactor);
    set_rect_frect(&pUIWND(&this->m_bk)->m_rect, &dlg_real_frect);
    set_rect_frect(&pUIWND(this)->m_rect, &dlg_real_frect);
    set_uiwnd_ptag(pUIWND(this), MAKE_PTAG(SF_SCENETEXT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_LOW, TR_SCALE_LOW));
    
    // calc timeclose and timeprogress rect
    fixui_pushstate(&dlg_old_frect, &dlg_real_frect, TR_CENTER, TR_HIGH, 1.0);
    fixui_adjust_RECT(&pUIWND(&this->timeclose)->m_rect, &pUIWND(&this->timeclose)->m_rect);
    fixui_adjust_RECT(&pUIWND(&this->timeprogress)->m_rect, &pUIWND(&this->timeprogress)->m_rect);
    fixui_popstate();
    
    // calc face area rect (not face rect itself)
    dlg_facearea_frect = dlg_frect;
    dlg_facearea_frect.bottom = sceneui_dstrect.bottom;
    
    // update face and textbox positions
    THISCALL_WRAPPER(UIRoleDialog_SetFace_wrapper, this, NULL, 0);
}

static MAKE_UIWND_RENDER_WRAPPER(UIRoleDialog_Render_wrapper, 0x004512F0)
static MAKE_UIWND_UPDATE_WRAPPER(UIRoleDialog_Update_wrapper, 0x004515E0)
static void fix_RoleDialog()
{
    // manually add wrapper to UIRoleDialog::Render/Update()
    INIT_WRAPPER_VFPTR(UIRoleDialog_Render_wrapper, 0x0056B150);
    INIT_WRAPPER_VFPTR(UIRoleDialog_Update_wrapper, 0x0056B154);
    
    // hook UIRoleDialog::Create
    INIT_WRAPPER_CALL(UIRoleDialog_Create_wrapper, { 0x0044F2E9 });
    
    // hook UIRoleDialog::SetFace
    INIT_WRAPPER_CALL(UIRoleDialog_SetFace_wrapper, {
        0x004515BB,
        0x00436B23,
        0x00451795,
    });
}





// fix YeTan timer
static MAKE_ASMPATCH(fix_YeTan_timer_hook_PrintString)
{
    fRECT src_frect, dst_frect;
    set_frect_ltrb(&src_frect, 690.0, 0.0, 800.0, 32.0);
    transform_frect(&dst_frect, &src_frect, &game_frect_original, &sceneui_dstrect, TR_HIGH, TR_LOW, sceneui_scalefactor);
    fixui_pushstate(&src_frect, &dst_frect, TR_SCALE_LOW, TR_SCALE_LOW, sceneui_scalefactor);
    
    R_ECX = R_ESI; // oldcode
    LINK_CALL(M_DWORD(R_EAX + 0x10));
}
static MAKE_ASMPATCH(fix_YeTan_timer_hook_Flush)
{
    fixui_popstate();
    
    struct gbPrintFont *pf = TOPTR(R_ESI);
    gbPrintFont_vfptr_Flush(pf); // same as old code
}
static void fix_YeTan_timer()
{
    // should push fixui state before PrintString()
    INIT_ASMPATCH(fix_YeTan_timer_hook_PrintString, 0x0040614B, 5, "\x8B\xCE\xFF\x50\x10");
    
    // should pop fixui state after PrintString() (i.e. before Flush())
    INIT_ASMPATCH(fix_YeTan_timer_hook_Flush, 0x00406150, 7, "\x8B\x16\x8B\xCE\xFF\x52\x08");
}




// fix encampment mini-game
static void pre_UIEncampment(struct UIWnd *this)
{
    fRECT dst_frect_area, dst_frect;
    get_ratio_frect(&dst_frect_area, &sceneui_dstrect, 4.0 / 3.0, TR_CENTER, TR_CENTER);
    transform_frect(&dst_frect_area, &dst_frect_area, &dst_frect_area, &sceneui_dstrect, TR_LOW, TR_LOW, 1.0);
    transform_frect(&dst_frect, &game_frect_original, &dst_frect_area, &dst_frect_area, TR_CENTER, TR_CENTER, ui_scalefactor);
    fixui_pushstate(&game_frect_original, &dst_frect, TR_SCALE_LOW, TR_SCALE_LOW, ui_scalefactor);
}
static void post_UIEncampment(struct UIWnd *this)
{
    fixui_popstate();
}
static MAKE_UIWND_RENDER_WRAPPER_CUSTOM(UIEncampment_Render_wrapper, 0x0052C450, pre_UIEncampment, post_UIEncampment)
static MAKE_UIWND_UPDATE_WRAPPER_CUSTOM(UIEncampment_Update_wrapper, 0x0052C480, pre_UIEncampment, post_UIEncampment)
static void fix_UIEncampment()
{
    // manually add wrapper to UIEncampment::Render/Update()
    INIT_WRAPPER_VFPTR(UIEncampment_Render_wrapper, 0x00570834);
    INIT_WRAPPER_VFPTR(UIEncampment_Update_wrapper, 0x00570838);
}


// fix skee mini-game
static MAKE_THISCALL(void, UISkee_Create_wrapper, struct UISkee *this)
{
    UISkee_Create(this);
    int i;
    struct uiwnd_ptag ptag;
    
    // fix dialog
    ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    set_uiwnd_ptag(pUIWND(&this->result_dlg), ptag);
    set_uiwnd_ptag(pUIWND(&this->help_dlg), ptag);
    
    // fix timer
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT_ORIGINAL, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->skeetimebk), ptag);
    for (i = 0; i < 10; i++) {
        set_uiwnd_ptag(pUIWND(&this->numberA[i]), ptag);
        set_uiwnd_ptag(pUIWND(&this->numberB[i]), ptag);
    }
}
static void pre_UISkee(struct UIWnd *this)
{
    fRECT dst_frect;
    get_ratio_frect(&dst_frect, &game_frect, 4.0 / 3.0, TR_CENTER, TR_CENTER);
    fixui_pushstate(&game_frect_original, &dst_frect, TR_SCALE_MID, TR_SCALE_MID, sceneicon_scalefactor);
}
static void post_UISkee(struct UIWnd *this)
{
    fixui_popstate();
}
static MAKE_UIWND_RENDER_WRAPPER_CUSTOM(UISkee_Render_wrapper, 0x0043DDF0, pre_UISkee, post_UISkee)
static MAKE_UIWND_UPDATE_WRAPPER_CUSTOM(UISkee_Update_wrapper, 0x00532710, pre_UISkee, post_UISkee)
static void fix_UISkee()
{
    INIT_WRAPPER_CALL(UISkee_Create_wrapper, { 0x0052EE78 });
    
    // manually add wrapper to UISkee::Render/Update()
    INIT_WRAPPER_VFPTR(UISkee_Render_wrapper, 0x00570894);
    INIT_WRAPPER_VFPTR(UISkee_Update_wrapper, 0x00570898);
}




// fix rowing game
static MAKE_THISCALL(bool, UIRowing_Create_wrapper, struct UIRowing *this)
{
    bool ret = UIRowing_Create(this);
    set_uiwnd_ptag(pUIWND(this), FIXUI_AUTO_TRANSFORM_PTAG);
    return ret;
}
static MAKE_UIWND_RENDER_WRAPPER(UIRowing_Render_wrapper, 0x00524970)
static MAKE_UIWND_UPDATE_WRAPPER(UIRowing_Update_wrapper, 0x005368A0)
static void fix_UIRowing()
{
    INIT_WRAPPER_CALL(UIRowing_Create_wrapper, { 0x0053529F });
    
    // manually add wrapper to UIRowing::Render/Update()
    INIT_WRAPPER_VFPTR(UIRowing_Render_wrapper, 0x005708EC);
    INIT_WRAPPER_VFPTR(UIRowing_Update_wrapper, 0x005708F0);
}



// fix UIGameOver
static MAKE_THISCALL(void, UIGameOver_Create_wrapper, struct UIGameOver *this)
{
    UIGameOver_Create(this);
    
    fRECT blood_frect;
    set_frect_rect(&blood_frect, &pUIWND(&this->m_Blood)->m_rect);
    transform_frect(&blood_frect, &blood_frect, &game_frect, &game_frect_original, TR_CENTER, TR_HIGH, 1.0);

    fRECT dst_frect;
    transform_frect(&dst_frect, &game_frect_original, &game_frect, &game_frect, TR_CENTER, TR_CENTER, scenetext_scalefactor);

    transform_frect(&blood_frect, &blood_frect, &game_frect_original, &dst_frect, TR_SCALE_LOW, TR_SCALE_LOW, scenetext_scalefactor);
    transform_frect(&blood_frect, &blood_frect, &game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, 1.0 / scenetext_scalefactor);
    set_rect_frect(&pUIWND(&this->m_Blood)->m_rect, &blood_frect);
    set_uiwnd_ptag(pUIWND(&this->m_Blood), MAKE_PTAG(SF_SCENETEXT, PTR_GAMERECT, PTR_GAMERECT, TR_SCALE_LOW, TR_SCALE_LOW));
}
static void fix_UIGameOver()
{
    INIT_WRAPPER_CALL(UIGameOver_Create_wrapper, {
        0x0044F99D,
        0x0044F9C3,
    });
}





// fix GrayScale
static MAKE_THISCALL(void, GrayScale_End_wrapper, struct GrayScale *this)
{
    fixui_pushidentity();
    fs->gb_align = 1;
    GrayScale_End(this);
    fixui_popstate();
}
static void fix_GrayScale()
{
    INIT_WRAPPER_CALL(GrayScale_End_wrapper, { 0x00405F84 });
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
    
    // fix YeTan mini-game timer
    fix_YeTan_timer();
    
    // fix encampment mini-game
    fix_UIEncampment();
    
    // fix skee mini-game
    fix_UISkee();
    
    // fix rowing game
    fix_UIRowing();
    
    // fix UIGameOver
    fix_UIGameOver();
    
    // fix GrayScale
    fix_GrayScale();
}
