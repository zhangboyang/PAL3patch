#include "common.h"

#define SCENE_PTAG(scalefactor, srcrect, lr, tb) MAKE_PTAG((scalefactor), (srcrect), sceneui_dstrect_type, (lr), (tb))

static int sceneui_dstrect_type;

// fix PlayerMgr: pushable notification, ShenYan numbers
static void __cdecl PlayerMgr_DrawMsg_hookpart1()
{
    set_uiwnd_ptag((struct UIWnd *) &g_msgbk, SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW));
    
    // step1: transform pushable notification
    push_ptag_state((struct UIWnd *) &g_msgbk);
    
    PlayerMgr_DrawMsg(); // oldcode, it will call part2
    
    // step3: restore fixui state stack
    fixui_popstate();
}
static void __fastcall PlayerMgr_DrawMsg_hookpart2(struct UIAnimateCtrl *this, int dummy)
{
    // step2: change fixui state, for g_jumpflag and ShenYan
    pop_ptag_state((struct UIWnd *) &g_msgbk);
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, sceneicon_scalefactor);
    
    UIAnimateCtrl_Render(this); // oldcode
}
static void install_PlayerMgr_DrawMsg_hook()
{
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
}
static void __fastcall UISceneMap_Render_hookpart2(struct UIWnd *this, int dummy)
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
static void __fastcall UIEmote_Render(struct UIEmote *this, int dummy)
{
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_HIGH, sceneicon_scalefactor);
    UIWnd_Render_rewrited((struct UIWnd *) this); // should call rewrited version
    fixui_popstate();
}


// general sceneui fixes
static MAKE_UIWND_RENDER_WRAPPER(ChoseCompose_Render_wrapper, 0x00524970)
static MAKE_UIWND_UPDATE_WRAPPER(ChoseCompose_Update_wrapper, 0x0047E3E0)
static MAKE_UIWND_RENDER_WRAPPER(UIRoleDialog_Render_wrapper, 0x004512F0)
static MAKE_UIWND_UPDATE_WRAPPER(UIRoleDialog_Update_wrapper, 0x004515E0)
static void __fastcall UIGameFrm_Create_wrapper(struct UIGameFrm *this, int dummy)
{
    UIGameFrm_Create(this);
    struct uiwnd_ptag ptag;
    
    ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    set_uiwnd_ptag((struct UIWnd *) &this->m_ChatRest, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_chosecompose, ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_note, ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag((struct UIWnd *) &this->m_seldlg, ptag);

    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, TR_SCALE_HIGH, TR_SCALE_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_cap, ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag((struct UIWnd *) &this->m_scenefrm->m_map, ptag);
    
    ptag = SCENE_PTAG(SF_SCENEUI, PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_scenefrm->m_face, ptag);
    

}




// fix RoleDialog
static int roledlg_facelrshift, roledlg_facetbshift;
static double roledlg_widthfactor, roledlg_heightfactor;
static double roledlg_widthdiff, roledlg_heightdiff;
static fRECT roledlg_old_frect, roledlg_new_frect;
static void __fastcall UIRoleDialog_Create_wrapper(struct UIRoleDialog *this, int dummy, int id, RECT *rect, struct UIWnd *pfather, const char *bkfile)
{
    UIRoleDialog_Create(this, id, rect, pfather, bkfile);
    
    struct uiwnd_ptag ptag;
    ptag = SCENE_PTAG(SF_SCENETEXT, PTR_GAMERECT_ORIGINAL, TR_CENTER, TR_HIGH);
    set_uiwnd_ptag((struct UIWnd *) this, ptag);
    
    // fix this->m_rect and this->m_bk.m_rect
    RECT *wndrect = &((struct UIWnd *) this)->m_rect;
    
    set_frect_rect(&roledlg_old_frect, wndrect);
    scale_frect_fixlt(&roledlg_new_frect, &roledlg_old_frect, roledlg_widthfactor, roledlg_heightfactor);
    transform_frect(&roledlg_new_frect, &roledlg_new_frect, &roledlg_new_frect, &roledlg_old_frect, TR_LOW, TR_HIGH, 1.0);
    set_rect_frect(wndrect, &roledlg_new_frect);
    set_rect_frect(&((struct UIWnd *)(&this->m_bk))->m_rect, &roledlg_new_frect);
    
    // calc parameters
    roledlg_heightdiff = get_frect_height(&roledlg_new_frect) - get_frect_height(&roledlg_old_frect);
    roledlg_widthdiff = get_frect_width(&roledlg_new_frect) - get_frect_width(&roledlg_old_frect);
    
    // fix this->timeprogress and this->timeclose
    fixui_pushstate(&roledlg_old_frect, &roledlg_new_frect, TR_CENTER, TR_HIGH, 1.0);
    fixui_adjust_RECT(&((struct UIWnd *)(&this->timeprogress))->m_rect, &((struct UIWnd *)(&this->timeprogress))->m_rect);
    fixui_adjust_RECT(&((struct UIWnd *)(&this->timeclose))->m_rect, &((struct UIWnd *)(&this->timeclose))->m_rect);
    fixui_popstate();
}
static void __fastcall UIRoleDialog_GetIconRect(struct UIRoleDialog *this, int dummy, RECT *rc, bool right)
{
    // fix arrow position
    fRECT frect;
    set_frect_ltrb(&frect, (right ? 760 : 18), 552, (right ? 792 : 50), 584);
    transform_frect(&frect, &frect, &roledlg_old_frect, &roledlg_new_frect, TR_HIGH, TR_HIGH, 1.0);
    set_rect_frect(rc, &frect); 
}
static void __fastcall UIRoleDialog_SetFace_wrapper(struct UIRoleDialog *this, int dummy, const char *path, int leftright)
{
    UIRoleDialog_SetFace(this, path, leftright);
    
    // fix m_face
    fRECT face_frect;
    set_frect_rect(&face_frect, &((struct UIWnd *)(&this->m_face))->m_rect);
    fRECT tmp_frect;
    tmp_frect.left = roledlg_new_frect.left; // lr => roledlg_new_frect
    tmp_frect.right = roledlg_new_frect.right;
    tmp_frect.top = game_frect_original.top; // tb => game_frect_original
    tmp_frect.bottom = game_frect_original.bottom;
    transform_frect(&face_frect, &face_frect, &game_frect, &tmp_frect, (leftright ? TR_HIGH : TR_LOW), TR_HIGH, 1.0);
    translate_frect_rel(&face_frect, &face_frect, (leftright ? -roledlg_facelrshift : roledlg_facelrshift), -roledlg_facetbshift);

    
    // fix m_static
    fRECT static_frect;
    set_frect_rect(&static_frect, &((struct UIWnd *)(&this->m_static))->m_rect);
    static_frect.bottom += roledlg_heightdiff;
    static_frect.left -= roledlg_widthdiff;
    
    // fix m_ani
    fRECT ani_frect;
    set_frect_rect(&ani_frect, &((struct UIWnd *)(&this->m_ani))->m_rect);
    
    // fix
    if (path) {
        if (leftright) { // right
            translate_frect_rel(&ani_frect, &ani_frect, -roledlg_facelrshift, 0);
            static_frect.right -= roledlg_facelrshift;
        } else { // left
            static_frect.left += roledlg_facelrshift;
        }
    } else { // noface
        // nothing to do
    }
    
    set_rect_frect(&((struct UIWnd *)(&this->m_face))->m_rect, &face_frect);
    set_rect_frect(&((struct UIWnd *)(&this->m_static))->m_rect, &static_frect);
    set_rect_frect(&((struct UIWnd *)(&this->m_ani))->m_rect, &ani_frect);
}
static void fix_RoleDialog()
{
    double dlgfactor = get_frect_min_scalefactor(get_ptag_frect(sceneui_dstrect_type), &game_frect_original);
    
    roledlg_facelrshift = 0;
    roledlg_facetbshift = 0;
    roledlg_widthfactor = dlgfactor / scenetext_scalefactor;
    roledlg_heightfactor = dlgfactor / scenetext_scalefactor;
    
    // hook UIRoleDialog::Create
    INIT_WRAPPER_CALL(UIRoleDialog_Create_wrapper, { 0x0044F2E9 });
    
    // rewrite UIRoleDialog::GetIconRect
    make_jmp(0x004517A0, UIRoleDialog_GetIconRect);
    
    // initialize roledlg_old_frect and roledlg_new_frect
    set_frect_ltrb(&roledlg_old_frect, 0, 0, 800, 600);
    set_frect_ltrb(&roledlg_new_frect, 0, 0, 800, 600);
    
    // hook UIRoleDialog::SetFace
    INIT_WRAPPER_CALL(UIRoleDialog_SetFace_wrapper, {
        0x004515BB,
        0x00436B23,
        0x00451795,
    });
}





MAKE_PATCHSET(fixsceneui)
{
    sceneui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixsceneui_scaletype"));
    sceneui_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_uiscalefactor"));
    sceneicon_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_iconscalefactor"));
    scenetext_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_textscalefactor"));

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

    // manually add wrapper to UIRoleDialog::Render/Update()
    INIT_WRAPPER_VFPTR(UIRoleDialog_Render_wrapper, 0x0056B150);
    INIT_WRAPPER_VFPTR(UIRoleDialog_Update_wrapper, 0x0056B154);

    // fix RoleDialog
    fix_RoleDialog();
}
