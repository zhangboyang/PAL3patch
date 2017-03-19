#include "stdafx.h"
#include "Resource.h"
#include "FontEnum.h"

class helper_arg {
public:
	std::vector<std::pair<CString, std::pair<CString, CString> > > *result;
};

static int CALLBACK EnumFontfaceHelper(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	helper_arg *arg = (helper_arg *) lParam;
	CString fontface(lpelfe->elfLogFont.lfFaceName);
	arg->result->push_back(std::make_pair(fontface, std::make_pair(fontface, EMPTYSTR)));
	return 1;
}
void EnumFontface(std::vector<std::pair<CString, std::pair<CString, CString> > > &result)
{
	result.clear();
	helper_arg arg;
	arg.result = &result;
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = '\0';
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx(GetDC(NULL), &lf, (FONTENUMPROC) EnumFontfaceHelper, (LPARAM) &arg, 0);
	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());
	result.insert(result.begin(), std::make_pair(CString(_T("default")), std::make_pair(STRTABLE(IDS_DEFAULTFONT), STRTABLE(IDS_DEFAULTFONT_DESC))));
}