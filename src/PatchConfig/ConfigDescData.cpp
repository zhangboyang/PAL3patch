#include "stdafx.h"


#define USE_CHS
#include "ConfigDescLocale.h"

//#define USE_CHT
//#include "ConfigDescLocale.h"

//#define USE_ENG
//#include "ConfigDescLocale.h"


static ConfigDescListIndex ConfigDescListArray[] = {
	{ _T("CHS"), ConfigDescList_CHS },
//	{ _T("CHT"), ConfigDescList_CHT },
//	{ _T("ENG"), ConfigDescList_ENG },
};


ConfigDescItem *ConfigDescList = NULL;

void LoadConfigDescList(LPCTSTR name)
{
	int i;
	int nlist = sizeof(ConfigDescListArray) / sizeof(ConfigDescListIndex);
	
	for (i = 0; i < nlist; i++) {
		if (_tcscmp(name, ConfigDescListArray[i].name) == 0) {
			ConfigDescList = ConfigDescListArray[i].list;
			return;
		}
	}

	ConfigDescList = NULL;
}
