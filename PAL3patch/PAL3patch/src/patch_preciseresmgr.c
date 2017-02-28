#include "common.h"

static MAKE_THISCALL(struct gbResource *, gbResManager_FindResByName, struct gbResManager *this, const char *resname)
{
    if (!resname) return NULL;
    unsigned hash = gbCrc32Compute(resname);
    int i;
    for (i = 0; i < this->CurNum; i++) {
        struct gbResource *cur = this->pBuffer[i];
        if (cur->NameCrc32 == hash && strcmp(cur->pName, resname) == 0) {
            cur->baseclass.RefCount++;
            return cur;
        }
    }
    return NULL;
}

MAKE_PATCHSET(preciseresmgr)
{
    make_jmp(gboffset + 0x100201B0, gbResManager_FindResByName);
}
