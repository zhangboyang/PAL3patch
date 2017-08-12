#include "stdafx.h"

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
	std::vector<std::pair<CString, std::pair<CString, CString> > > buf;
	std::vector<std::pair<CString, std::pair<CString, CString> > >::iterator it;
	helper_arg arg;
	arg.result = &buf;
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = '\0';
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx(GetDC(NULL), &lf, (FONTENUMPROC) EnumFontfaceHelper, (LPARAM) &arg, 0);
	std::sort(buf.begin(), buf.end());
	buf.erase(std::unique(buf.begin(), buf.end()), buf.end());
	result.clear();
	for (it = buf.begin(); it != buf.end(); it++) {
		if (it->first >= CString(_T("\x80"))) {
			result.push_back(*it);
		}
	}
	for (it = buf.begin(); it != buf.end(); it++) {
		if (it->first < CString(_T("\x80"))) {
			result.push_back(*it);
		}
	}
	result.insert(result.begin(), std::make_pair(CString(_T("default")), std::make_pair(STRTABLE(IDS_DEFAULTFONT), STRTABLE(IDS_DEFAULTFONT_DESC))));
}