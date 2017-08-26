#include "PAL3patch.h"

#pragma comment(lib, "PAL3patch.lib")

extern int init_voice_adapter();

MAKE_PLUGINENTRY()
{
	return init_voice_adapter();
}
