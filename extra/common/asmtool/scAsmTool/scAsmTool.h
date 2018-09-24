#pragma once
enum {
	LVL_FATAL,
	LVL_ERROR,
	LVL_WARN,

	LVL_MAX // end
};

class scAsmTool {
	static std::string fn;
	static std::string line;
	static int ln, col, ecol;
	static int msgcnt[LVL_MAX];

	static void DumpLine();
public:
	

	static void SetFile(const std::string &fn);
	static void SetLine(const std::string &line);
	static void SetLoc(int ln, int col, int ecol);
	static void ResetLoc();

	static void ReportError(int level, const char *fmt, ...);
	static void ReportError(int level, const char *fmt, va_list ap);
	
	static int GetErrorCnt();
	static int GetWarnCnt();

	__declspec(noreturn) static void die(int exitcode);
	static std::string cmdline;
	static bool dbgflag;
};

