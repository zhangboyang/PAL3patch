#include "common.h"

static int waittype;
static void flush_buffered_frames()
{
    IDirect3DQuery9 *pQuery;
    if (IDirect3DDevice9_CreateQuery(g_GfxMgr->m_pd3dDevice, D3DQUERYTYPE_EVENT, &pQuery) == D3D_OK) {
        if (pQuery) {
            IDirect3DQuery9_Issue(pQuery, D3DISSUE_END);
            while (IDirect3DQuery9_GetData(pQuery, NULL, 0, D3DGETDATA_FLUSH) == S_FALSE) {
                if (waittype == 2) {
                    Sleep(1);
                }
            }
            IDirect3DQuery9_Release(pQuery);
        }
    }
}

MAKE_PATCHSET(reduceinputlatency)
{
    waittype = flag;
    add_postpresent_hook(flush_buffered_frames);
}
