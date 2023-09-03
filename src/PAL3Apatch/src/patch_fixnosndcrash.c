#include "common.h"

static MAKE_THISCALL(struct gbAudioManager *, gbAudioManager_Ctor_wrapper, struct gbAudioManager *this, int *succeed, struct gbAudioMgrDesc *pdesc)
{
    memset(this, 0, sizeof(*this));
    return gbAudioManager_Ctor(this, succeed, pdesc);
}

MAKE_PATCHSET(fixnosndcrash)
{
    make_branch(0x0052876A, 0xE8, gbAudioManager_Ctor_wrapper, 6);
}
