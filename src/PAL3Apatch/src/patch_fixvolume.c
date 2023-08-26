#include "common.h"

static MAKE_THISCALL(void, UIDragBar_SetMaxVal_wrapper, struct UIDragBar *this, float val)
{
    UIDragBar_SetMaxVal(this, 1.0);
}

MAKE_PATCHSET(fixvolume)
{
    INIT_WRAPPER_CALL(UIDragBar_SetMaxVal_wrapper, { 0x004A3927, 0x004A38C6 });
    
    static float snd_volume, music_volume;
    
    const char *volume_cfgstr = get_string_from_configfile("defaultvolume");
    if (sscanf(volume_cfgstr, "%f,%f", &snd_volume, &music_volume) != 2) {
        fail("invalid default volume config string '%s'.", volume_cfgstr);
    }
    
    make_pointer(0x0052883A + 2, &snd_volume);
    make_pointer(0x00528853 + 2, &music_volume);
    memcpy_to_process(0x00528859 + 6, &music_volume, sizeof(float));
}
