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
    PATCH_FLOAT_MEMREF_EXPR(scale_gbxdiff(0.1f, sceneicon_scalefactor), { 0x00403985 });
}
static void fix_scene_lostblood()
{
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
static MAKE_ASMPATCH(UISceneMap_Render_hookpart1)
{
    // step1: transform rect and push identity
    RECT *rc = TOPTR(R_EBP - 0x14);
    fixui_adjust_RECT(rc, rc);
    fixui_pushidentity();
    fs->gb_align = 1;
    
    R_ESI += 0x0010AF24; // oldcode
}
static MAKE_ASMPATCH(UISceneMap_Render_fixui_stack_balance)
{
    fixui_pushidentity();
    
    M_DWORD(R_EBX + 0x4B4) = R_ESI; // oldcode
}
static MAKE_THISCALL(void, UISceneMap_Render_hookpart2, struct UIWnd *this)
{
    // step2: restore fixui state
    fixui_popstate();
    UIWnd_Render_rewrited(this); // should call rewrited version
}

static MAKE_THISCALL(void, UISceneMap_Render_fixroleicon_gbMatrixStack_Translate_wrapper, struct gbMatrixStack *this, float x, float y, float z)
{
    gbMatrixStack_Translate(this, x, y, z);
    gbMatrixStack_Scale(this, sceneui_scalefactor, sceneui_scalefactor, 1.0);
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
    
    // hookpart2 is an wrapper to UIWnd::Render()
    INIT_WRAPPER_CALL(UISceneMap_Render_hookpart2, { 0x0045930E });
    
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
    set_uiwnd_ptag(pUIWND(&this->m_selemote_frame), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_selemote_BTN1), ptag);
    set_uiwnd_ptag(pUIWND(&this->m_selemote_BTN2), ptag);
    
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_note), ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag(pUIWND(&this->m_seldlg), ptag);
    
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_SCALE_MID, TR_SCALE_MID);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, TR_SCALE_HIGH, TR_SCALE_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_cap), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_map), ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag(pUIWND(&this->m_scenefrm->m_face), ptag);
}


static void fix_gamescene()
{
    // hook UIGameFrm::Create()
    INIT_WRAPPER_CALL(UIGameFrm_Create_wrapper, {
        0x00406A73,
        0x00406AC2,
    });
    
    // hook PlayerMgr::DrawMsg()
    install_PlayerMgr_DrawMsg_hook();
    
    // hook UISceneMap::Render()
    install_UISceneMap_Render_hook();
    
    // hook UIEmote::Render()
    INIT_WRAPPER_VFPTR(UIEmote_Render, 0x00558FD0);
    
    // fix ChatRest dialog position
    SIMPLE_PATCH(0x00450394, "\x05\x2C\x01\x00\x00", "\x05\xFA\x00\x00\x00", 5);
}


#if 0

// fix RoleDialog
#define ROLEDLG_FACESIZE 2.0 // ratio of text frame
#define ROLEDLG_FACEHEIGHTFACTOR (256.0 / 512.0) // real face height : face texture height
static double dlg_minsize; // ratio of whole screen
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
    fRECT ideal_textarea_frect;
    fRECT new_text_frect;
    set_frect_ltrb(&ideal_textarea_frect, dlg_real_frect.left, textarea_frect.top, dlg_real_frect.right, textarea_frect.bottom);
    transform_frect(&new_text_frect, &text_frect, &ideal_textarea_frect, &ideal_textarea_frect, TR_CENTER, TR_CENTER, 1.0);
    
    if (new_text_frect.left < textarea_frect.left) {
        translate_frect_rel(&new_text_frect, &new_text_frect, textarea_frect.left - new_text_frect.left, 0.0);
    } else if (new_text_frect.right > textarea_frect.right) {
        translate_frect_rel(&new_text_frect, &new_text_frect, textarea_frect.right - new_text_frect.right, 0.0);
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
/*static MAKE_ASMPATCH(dlgfillchar)
{
    static char *s = NULL;
    if (!s) {
        int sz = 1000;
        s = malloc(sz + 1);
        s[sz] = 0;
        memset(s, 'A', sz);
    }
    R_ECX = R_EBX;
    R_EAX = TOUINT(s);
}*/
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

    // fill dlgtext, for debug purpose
    //INIT_ASMPATCH(dlgfillchar, 0x00451215, 6, "\x8B\x44\x24\x30\x8B\xCB");
}


#endif






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



MAKE_PATCHSET(fixsceneui)
{
    sceneui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixsceneui_scaletype"));
    sceneui_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_uiscalefactor"));
    sceneicon_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_iconscalefactor"));
    scenetext_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_textscalefactor"));
    
    #if 0
    dlg_minsize = str2double(get_string_from_configfile("fixsceneui_dlgminsize"));
    #endif

    // general fixes
    fix_gamescene();
    
    #if 0
    // fix RoleDialog
    fix_RoleDialog();
    #endif
    
    // fix UIGameOver
    fix_UIGameOver();
}
