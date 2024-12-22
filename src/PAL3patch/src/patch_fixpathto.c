#include "common.h"

static int suppress = 0;
static int failed = 0;
static int fps;

static MAKE_THISCALL(void, Role_OnBehaviourFinish_wrapper, struct Role *this)
{
	if (suppress) {
		failed = 1;
		return;
	}
	Role_OnBehaviourFinish(this);
}

static MAKE_THISCALL(void, Role_UpdateMove_wrapper, struct Role *this, float deltatime, bool autopos)
{
	if (autopos) {
		struct gbVec3D m_wanapos_sv;
		struct gbVec3D m_lastautodir_sv;
		memcpy(&m_wanapos_sv, &this->m_wanapos, sizeof(struct gbVec3D));
		memcpy(&m_lastautodir_sv, &this->m_lastautodir, sizeof(struct gbVec3D));
		suppress = 1;
		failed = 0;
		Role_UpdateMove(this, deltatime, autopos);
		suppress = 0;
		if (!failed) return;
		memcpy(&this->m_wanapos, &m_wanapos_sv, sizeof(struct gbVec3D));
		memcpy(&this->m_lastautodir, &m_lastautodir_sv, sizeof(struct gbVec3D));
		deltatime = 1.0 / fps;
	}
	Role_UpdateMove(this, deltatime, autopos);
}

MAKE_PATCHSET(fixpathto)
{
	fps = flag;

	INIT_WRAPPER_CALL(Role_UpdateMove_wrapper, {
		0x0041216B,
		0x004121D8,
		0x00417E24,
		0x00417EE7,
	});

	INIT_WRAPPER_CALL(Role_OnBehaviourFinish_wrapper, {
		0x004148AA, // Role::AutoPosition -> Role::OnBehaviourFinish
	});
}
