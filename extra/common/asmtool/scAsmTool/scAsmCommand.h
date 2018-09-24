#pragma once

#define SC_MAX_PARAM 7

#define SC_MAX_CMD 512

#define VAR_NONE 255
#define VAR_INT 1
#define VAR_FLOAT 2
#define VAR_STRING 3
#define VAR_JMPFLAG 4

#define VAR_USERVAR 5

#define VAR_LIST 250 //only for CmdDef param type,not true var type

struct scCmdDef {
	const char *pfun;
	BYTE paramlist[SC_MAX_PARAM];//param type list,paramlist[0]==VAR_LIST for variety param list
	WORD paramnum;

	const char *GetCmdName();
	static int GetCmdID(const char *cmdname);
	static scCmdDef &GetCmdDef(int id);
	static bool IsLegalInstr(int id, WORD paramflag);
};

#define REGISTER_CMD(x) const char x[] = #x

extern scCmdDef g_CmdDefArray[];

extern void InitCmdDef_PAL3();
extern void InitCmdDef_PAL3A();
