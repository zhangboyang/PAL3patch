#include "common.h"

static MAKE_THISCALL(struct gbAudioManager *, gbAudioManager_Ctor_wrapper, struct gbAudioManager *this, int *succeed, struct gbAudioMgrDesc *pdesc)
{
    memset(this, 0, sizeof(*this));
    return gbAudioManager_Ctor(this, succeed, pdesc);
}

MAKE_PATCHSET(fixnosndcrash)
{
    make_call6(0x0053DDD7, gbAudioManager_Ctor_wrapper);
}
