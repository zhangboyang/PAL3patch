#include "stdafx.h"

const char pBuildDate[] = __DATE__ ", " __TIME__;
const char pVersionStr[] = PATCH_VERSION_CSTR;
const char pCompiler[] = "Microsoft C " TOSTR(_MSC_VER);

const char *pFileHash[] = {
#ifndef _DEBUG

	// format: filename, sha1 hash,

	// PAL3 executables
	"PAL3.EXE",          "05e1cbd148957dfb635d291c13f19105133ba875",
	"GBENGINE.DLL",      "ead9bba8ce18fb68f111c1cb1edbf30ddb78b367",
	"PAL3UNPACK.DLL",    "7b04cded42cd388f7e4493fa8588c885dc6e3d4b",

	// MSS32 version 6.1c
	"Mss32.dll",                  "2ee3fffdb4a5c4b9c7da106f07506d0d1685b395",
	"miles\\win32\\Autopan.flt",  "c30199ee9a8ca918c19afbd440ce8370d1295523",
	"miles\\win32\\Bandpass.flt", "1b2dd894019d1c7662f0451b825cd4cfd4495936",
	"miles\\win32\\Capture.flt",  "18e836566d9b9607c250c828e0dc2da04d873ce7",
	"miles\\win32\\Chorus.flt",   "ed288c3564624d4ab6e64024fdfa03d977b14ab9",
	"miles\\win32\\Compress.flt", "48d37935027823d9dd1a2a26084761355bdee4b1",
	"miles\\win32\\Flange.flt",   "fff146199c8e766b0ca2c4fe0fe1c47342a4084c",
	"miles\\win32\\Highpass.flt", "cfc68f79095d9408f94066bb4a1c3bbfab1aeb56",
	"miles\\win32\\Laginter.flt", "50421450022d94550e2b50741c0bec37bb83ef6e",
	"miles\\win32\\Lowpass.flt",  "3e3638f3a2c5c47ba95704305aef52259f238308",
	"miles\\win32\\Mdelay.flt",   "0a0d8da94134a4a75c27785589f0bd33ceabbfe7",
	"miles\\win32\\Mp3dec.asi",   "62b8bd50d8145e4b833bf05fa77375efb2008cce",
	"miles\\win32\\Mss32.dll",    "2ee3fffdb4a5c4b9c7da106f07506d0d1685b395",
	"miles\\win32\\Mssa3d.m3d",   "89a3f65ba18ce6f080e45b2e469160b99e2dd231",
	"miles\\win32\\Mssa3d2.m3d",  "17d25e3ed527a8b891a16660067b84d61db5ab8a",
	"miles\\win32\\Mssdolby.m3d", "dd7c2d79fc28cc2a66aa2aff77fb9509d3223f33",
	"miles\\win32\\Mssds3dh.m3d", "a5200ac03a8ff04dc2069598d9941be6a20ea665",
	"miles\\win32\\Mssds3ds.m3d", "53bd94224f8823335accd74b9d8d6d8eb6449559",
	"miles\\win32\\Mssdx7sh.m3d", "ab753d2f897a6164ddd2669b8c5aab718ff3e1de",
	"miles\\win32\\Mssdx7sl.m3d", "826919027c5a8dafbf3fd5943023e83998c3e5aa",
	"miles\\win32\\Mssdx7sn.m3d", "25b659e831da762d0d9a123da3e4ba6cdf5f7bb6",
	"miles\\win32\\Msseax.m3d",   "97b25166c3488d11f1846c4f0860cfec312d23b2",
	"miles\\win32\\Msseax2.m3d",  "efc3e79c40bc83447be25e8b141642c81e4ad2c3",
	"miles\\win32\\Mssfast.m3d",  "78e0777c615b8b9315dcff4b6b896e12b53b9dae",
	"miles\\win32\\Mssrsx.m3d",   "2ea186a61679d7d2ab9fbf793330c629349431ba",
	"miles\\win32\\Mssv12.asi",   "27c827e4af491e1f0ea4d8174f7eee3c349e6241",
	"miles\\win32\\Mssv24.asi",   "f9b113d071a636eca17a7ff16b3f16973522d868",
	"miles\\win32\\Mssv29.asi",   "d5780ca487ed0bfda78cfe32614e79b6005f4cd3",
	"miles\\win32\\Parmeq.flt",   "7c9a3fe4e30c948629b2f53652e4046155a9205a",
	"miles\\win32\\Phaser.flt",   "3304bc9a6a9365799ebf6809b9a2537fddda8220",
	"miles\\win32\\Reson.flt",    "15183e6ed84d1807101b1b6748c08e5ec490dbb7",
	"miles\\win32\\Reverb1.flt",  "86fc7061dfc3e336ff5bc98a14650cf736ea98c2",
	"miles\\win32\\Reverb2.flt",  "c0d7cab5ab0dd7152f25a88c48c75f1bf2318c2b",
	"miles\\win32\\Reverb3.flt",  "e98400d77c6260be78e74a2c31245a8f61f5e0d7",
	"miles\\win32\\Ringmod.flt",  "62afbd1eae48ad6ff6b84244b29648c48d1562be",
	"miles\\win32\\Sdelay.flt",   "d002f287d0208780d0adadf7e7cff049dccb7af3",
	"miles\\win32\\Shelfeq.flt",  "4ea2aabdd630e47fdaa804a2a10398f0a3f8c4f8",

#endif
	// EOF
	NULL,
};
