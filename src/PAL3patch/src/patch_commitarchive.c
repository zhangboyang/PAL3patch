#include "common.h"

static int fclose_wrapper(FILE *fp)
{
    pal3fflush(fp);
    return pal3fclose(fp);
}

MAKE_PATCHSET(commitarchive)
{
    make_pointer(0x0053860B, "wbc");
    make_call(0x00538C9E, fclose_wrapper);
}
