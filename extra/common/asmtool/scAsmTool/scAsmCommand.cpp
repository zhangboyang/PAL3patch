#include "common.h"

scCmdDef g_CmdDefArray[SC_MAX_CMD];

const char *scCmdDef::GetCmdName()
{
	const char *cmdname = this->pfun;
	if (cmdname && strncmp(cmdname, "cmd_", 4) == 0) cmdname += 4;
	return cmdname;
}
int scCmdDef::GetCmdID(const char *cmdname)
{
	for (int i = 0; i < SC_MAX_CMD; i++) {
		scCmdDef &cmddef = GetCmdDef(i);
		const char *curcmdname = cmddef.GetCmdName();
		if (curcmdname == NULL) continue;
		if (stricmp(curcmdname, cmdname) == 0) return i;
	}
	return -1;
}
scCmdDef &scCmdDef::GetCmdDef(int id)
{
	assert(0 <= id && id < SC_MAX_CMD);
	return g_CmdDefArray[id];
}
bool scCmdDef::IsLegalInstr(int id, WORD paramflag)
{
	if (!(0 <= id && id < SC_MAX_CMD)) {
		return false;
	}

	scCmdDef &cmddef = GetCmdDef(id);

	if (!cmddef.pfun) return false;

	WORD mask = 0;
	for (int i = 0; i < (int) cmddef.paramnum; i++) {
		mask |= (1 << i);
	}
	if ((paramflag & mask) != paramflag) {
		return false;
	}

	return true;
}