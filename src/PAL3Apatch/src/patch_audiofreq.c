#include "common.h"

static int freq;

static MAKE_THISCALL(struct gbAudioMgrDesc *, gbAudioMgrDesc_Ctor, struct gbAudioMgrDesc *this)
{
    this->frequence = freq;
    this->bits = 16;
    this->channels = 2;
    this->provider[0] = 0;
    return this;
}

MAKE_PATCHSET(audiofreq)
{
    freq = flag;
    make_jmp(gboffset + 0x10001000, gbAudioMgrDesc_Ctor);
}
