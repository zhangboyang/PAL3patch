#include "common.h"

int scAsmTool::ln = -1;
int scAsmTool::col = -1;
int scAsmTool::ecol = -1;
std::string scAsmTool::fn("<unknown>");
std::string scAsmTool::line;
int scAsmTool::msgcnt[LVL_MAX] = {};
bool scAsmTool::dbgflag = false;

void scAsmTool::SetLine(const std::string &line)
{
	scAsmTool::line = line;
}
void scAsmTool::SetFile(const std::string &fn)
{
	scAsmTool::fn = fn;
}
void scAsmTool::SetLoc(int ln, int col, int ecol)
{
	scAsmTool::ln = ln;
	scAsmTool::col = col;
	scAsmTool::ecol = ecol;
}
void scAsmTool::ResetLoc()
{
	ln = col = ecol = -1;
}

void scAsmTool::DumpLine()
{
	if (ln < 0 || col < 0 || ecol < 0) return;
	int len = printf("%s(%d):", fn.c_str(), ln + 1);
	printf("%s\n", line.c_str());
	printf("%*s", len, "");
	bool flag = false;
	for (size_t i = 0; i < line.size(); i++) {
		if (i == col || i == ecol) {
			if (i == col) flag = true;
			if (i == ecol) flag = false;
			printf("^");
		} else {
			if (flag) printf("~"); else printf(" ");
		}
	}
	printf("\n");
}

void scAsmTool::ReportError(int level, const char *fmt, va_list ap)
{
	DumpLine();
	msgcnt[level]++;
	switch (level) {
		case LVL_FATAL: printf("  ÑÏÖØ´íÎó: "); break;
		case LVL_ERROR: printf("  ´íÎó: "); break;
		case LVL_WARN: printf("  ¾¯¸æ: "); break;
	}
	vprintf(fmt, ap);
	printf("\n\n");
	if (level == LVL_FATAL) die(1);
}
void scAsmTool::ReportError(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ReportError(level, fmt, ap);
	va_end(ap);
}

int scAsmTool::GetErrorCnt()
{
	return msgcnt[LVL_FATAL] + msgcnt[LVL_ERROR];
}
int scAsmTool::GetWarnCnt()
{
	return msgcnt[LVL_WARN];
}

__declspec(noreturn) void scAsmTool::die(int exitcode)
{
	//assert(exitcode == 0);
	if (!exitcode) printf(" -- ³É¹¦ --");
	if (exitcode) printf(" -- Ê§°Ü£¨´úÂë %d£© --", exitcode);
	printf(" £¨%d ´íÎó£¬%d ¾¯¸æ£©\n", GetErrorCnt(), GetWarnCnt());
	if (exitcode) system("pause");

	exit(exitcode);
}

std::string scAsmTool::cmdline;