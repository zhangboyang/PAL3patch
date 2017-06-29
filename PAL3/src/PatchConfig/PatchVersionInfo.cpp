#include "stdafx.h"
#include "PatchVersionInfo.h"

const char pBuildDate[] = __DATE__ ", " __TIME__;
const char pVersionStr[] = PATCH_VERSION;

const char *pFileHash[] = {
	// filename, sha1 hash,
	"PAL3.EXE",          "05e1cbd148957dfb635d291c13f19105133ba875",
	"GBENGINE.DLL",      "ead9bba8ce18fb68f111c1cb1edbf30ddb78b367",
	"PAL3UNPACK.DLL",    "7b04cded42cd388f7e4493fa8588c885dc6e3d4b",

	// EOF
	NULL,
};
