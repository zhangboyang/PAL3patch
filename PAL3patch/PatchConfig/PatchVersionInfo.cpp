#include "stdafx.h"
#include "PatchVersionInfo.h"


#define PATCH_VERSION "v1.0 alpha"
#define PATCH_VERSION_WSTR _T(PATCH_VERSION)


const TCHAR pWelcomeMessage_CHS[] = 
	_T("欢迎使用《仙剑奇侠传三》增强补丁 ") PATCH_VERSION_WSTR _T("\r\n")
	_T("本补丁可以修复一些游戏程序的小问题，\r\n")
	_T("并且添加了一些实用的小功能。\r\n")
	_T("详细信息和使用方法请参见补丁说明文件。")
;

LPCTSTR pWelcomeMessage = pWelcomeMessage_CHS;

const char pBuildDate[] = __DATE__ ", " __TIME__;
const char pVersionStr[] = PATCH_VERSION;

const char *pFileHash[] = {
	// filename, sha1 hash,
	"PAL3.EXE",     "05e1cbd148957dfb635d291c13f19105133ba875",
	"GBENGINE.DLL", "ead9bba8ce18fb68f111c1cb1edbf30ddb78b367",
	// EOF
	NULL,
};
