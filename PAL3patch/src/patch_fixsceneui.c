#include "common.h"

#define SCENE_PTAG(srcrect, lr, tb) MAKE_PTAG(SF_SCENEUI, (srcrect), sceneui_dstrect_type, (lr), (tb))

static int sceneui_dstrect_type;

// fix PlayerMgr: pushable notification, ShenYan numbers
static void __cdecl PlayerMgr_DrawMsg_hookpart1()
{
    set_uiwnd_ptag((struct UIWnd *) &g_msgbk, SCENE_PTAG(PTR_GAMERECT, TR_CENTER, TR_LOW));
    
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
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_LOW, TR_SCALE_LOW, sceneui_scalefactor);
    
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
    fixui_pushstate(&game_frect, &game_frect, TR_SCALE_MID, TR_SCALE_HIGH, sceneui_scalefactor);
    UIWnd_Render_rewrited((struct UIWnd *) this); // should call rewrited version
    fixui_popstate();
}


// general fixes
static MAKE_UIWND_RENDER_WRAPPER(ChoseCompose_Render_wrapper, 0x00524970)
static MAKE_UIWND_UPDATE_WRAPPER(ChoseCompose_Update_wrapper, 0x0047E3E0)
static void __fastcall UIGameFrm_Create_wrapper(struct UIGameFrm *this, int dummy)
{
    UIGameFrm_Create(this);
    struct uiwnd_ptag ptag;
    
    ptag = FIXUI_AUTO_TRANSFORM_PTAG;
    set_uiwnd_ptag((struct UIWnd *) &this->m_ChatRest, ptag);
    set_uiwnd_ptag((struct UIWnd *) this->m_chosecompose, ptag);

    ptag = SCENE_PTAG(PTR_GAMERECT, TR_CENTER, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_note, ptag);

    ptag = SCENE_PTAG(PTR_GAMERECT, TR_CENTER, TR_CENTER);
    set_uiwnd_ptag((struct UIWnd *) &this->m_seldlg, ptag);

    ptag = SCENE_PTAG(PTR_GAMERECT_ORIGINAL, TR_SCALE_HIGH, TR_SCALE_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_cap, ptag);
    
    ptag = SCENE_PTAG(PTR_GAMERECT, TR_LOW, TR_HIGH);
    set_uiwnd_ptag((struct UIWnd *) &this->m_scenefrm->m_map, ptag);
    
    ptag = SCENE_PTAG(PTR_GAMERECT, TR_HIGH, TR_LOW);
    set_uiwnd_ptag((struct UIWnd *) &this->m_scenefrm->m_face, ptag);
}


MAKE_PATCHSET(fixsceneui)
{
    sceneui_scalefactor = str2scalefactor(get_string_from_configfile("fixsceneui_scalefactor"));
    sceneui_dstrect_type = parse_uiwnd_rect_type(get_string_from_configfile("fixsceneui_scaletype"));

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
    
    // manually add wrapper to ChoseCompose::Render()
    INIT_WRAPPER_VFPTR(ChoseCompose_Render_wrapper, 0x0056B8A8);
    INIT_WRAPPER_VFPTR(ChoseCompose_Update_wrapper, 0x0056B8AC);
}
