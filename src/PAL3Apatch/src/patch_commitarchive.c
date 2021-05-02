#include "common.h"

static int fclose_wrapper(FILE *fp)
{
    pal3afflush(fp);
    return pal3afclose(fp);
}

MAKE_PATCHSET(commitarchive)
{
    make_pointer(0x0052330A, "wbc");
    make_call(0x00523691, fclose_wrapper);
}
