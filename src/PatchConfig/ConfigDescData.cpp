#include "stdafx.h"


bool ConfigDescOptionListEnum::TranslateSelectedValue(CWnd *listWnd, CString &value)
{
	return true;
}
bool ConfigDescOptionListEnum::IsValueEqual(const CString &lstval, const CString &selval)
{
	return lstval == selval;
}
bool ConfigDescOptionListEnum::IsValueLegal(const std::vector<CString> &lst, const CString &selval)
{
	std::vector<CString>::const_iterator it;
	for (it = lst.begin(); it != lst.end(); it++) {
		if (IsValueEqual(*it, selval)) {
			return true;
		}
	}
	return false;
}
CString ConfigDescOptionListEnum::GetValueTitle(const CString &value)
{
	return value;
}
CString ConfigDescOptionListEnum::GetValueDescription(const CString &value)
{
	return GetValueTitle(value);
}
CString ConfigDescOptionListEnum::GetFallbackValue()
{
	return CString();
}


#define USE_CHS
#include "ConfigDescLocale.h"

#define USE_CHT
#include "ConfigDescLocale.h"

static ConfigDescListIndex ConfigDescListArray[] = {
	{ _T("CHS"), ConfigDescList_CHS },
	{ _T("CHT"), ConfigDescList_CHT },
};


ConfigDescItem *ConfigDescList = NULL;
bool ConfigDirty;

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
