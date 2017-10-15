#include "common.h"

static int lastSlot1[11], lastSlot2[11];
static double fSlot1[11], fSlot2[11];

static MAKE_THISCALL(void, CCBAttackSequen_RunOneStep, struct CCBAttackSequen *this)
{
    int i;
    for (i = 0; i < 11; i++) {
        struct tagAttackSequen *cur = &this->m_Sequen[i];
        if (cur->bValid && CCBRoleState_IsAlive(this->m_pCBSystem->m_pRoleState, i)) {
            if (cur->bSlot2Running) {
                if (lastSlot2[i] != cur->nSlot2) fSlot2[i] = lastSlot2[i] = cur->nSlot2;
                fSlot2[i] = cur->fDeltaTime * 50.0 * cur->bySpeed2 + fSlot2[i];
                cur->nSlot2 = lastSlot2[i] = floor(fSlot2[i]);
                if (cur->nSlot2 > 11500) {
                    fSlot2[i] = lastSlot2[i] = cur->nSlot2 = 11500;
                }
            } else {
                if (lastSlot1[i] != cur->nSlot1) fSlot1[i] = lastSlot1[i] = cur->nSlot1;
                int old_nSlot1 = cur->nSlot1;
                fSlot1[i] = cur->fDeltaTime * 50.0 * cur->bySpeed1 + fSlot1[i];
                cur->nSlot1 = lastSlot1[i] = floor(fSlot1[i]);
                if (cur->nSlot1 >= 11500) {
                    if (old_nSlot1 < 11500) cur->nRound++;
                    fSlot1[i] = lastSlot1[i] = cur->nSlot1 = 11500;
                }
            }
        }
    }
}

MAKE_PATCHSET(fixattacksequen)
{
    make_jmp(0x004CF221, CCBAttackSequen_RunOneStep);
}
