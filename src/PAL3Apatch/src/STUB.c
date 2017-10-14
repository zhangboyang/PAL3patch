#include "common.h"

HWND game_hwnd;
fRECT game_frect = {0, 0, 800, 600};

int get_showcursor_state()
{
    // see UICursor::Show() for details
    if (UICursor_Inst()->m_bSoftMode) {
        return !!UICursor_Inst()->m_bShow;
    } else {
        return !!GB_GfxMgr->m_bShowCursor;
    }
}
void set_showcursor_state(int show)
{
    UICursor_Show(UICursor_Inst(), !!show);
}
