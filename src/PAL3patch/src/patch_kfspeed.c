#include "common.h"

static int kfspeed = 1;

static MAKE_THISCALL(int, UIKillFlyer_Update_wrapper, struct UIKillFlyer *this, float deltatime, int haveinput)
{
    int i;
    for (i = 1; i <= kfspeed; i++) {
        int ret = UIKillFlyer_Update(this, deltatime, haveinput);
        if (ret) haveinput = 0;
    }
    return !haveinput;
}

MAKE_PATCHSET(kfspeed)
{
    kfspeed = flag;
    INIT_WRAPPER_VFPTR(UIKillFlyer_Update_wrapper, 0x00570774);
}
