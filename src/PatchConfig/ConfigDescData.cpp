#include "stdafx.h"


bool ConfigDescOptionListEnum::TranslateSelectedValue(HWND hWnd, CString &value)
{
	return true;
}
bool ConfigDescOptionListEnum::IsValueEqual(const CString &lstval, const CString &selval)
{
	return lstval == selval;
}
CString ConfigDescOptionListEnum::GetValueTitle(const CString &value)
{
	return value;
}
CString ConfigDescOptionListEnum::GetValueDescription(const CString &value)
{
	return GetValueTitle(value);
}

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
