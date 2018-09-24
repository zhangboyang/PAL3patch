#pragma once

#define SCE_VERSION 1
#pragma pack(1)
	struct SceHeader
	{
		BYTE flag[4];
		BYTE version;
		WORD blocknum;
	};

	struct SceIndex
	{
		DWORD id;
		DWORD offset;
		char  desc[64];
	};

#pragma pack()

