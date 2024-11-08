#include "stdafx.h"


EnumFontface EnumFontfaceInstance;

static int CALLBACK EnumFontfaceHelper(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	EnumFontface *arg = (EnumFontface *) lParam;
	arg->buf.push_back(CString(lpelfe->elfLogFont.lfFaceName));
	return 1;
}

void EnumFontface::EnumConfigValues(std::vector<CString> &result)
{
	std::vector<CString>::iterator it;
	TCHAR ch;
	buf.clear();
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = '\0';
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx(GetDC(NULL), &lf, (FONTENUMPROC) EnumFontfaceHelper, (LPARAM) this, 0);
	std::sort(buf.begin(), buf.end());
	buf.erase(std::unique(buf.begin(), buf.end()), buf.end());
	result.clear();
	result.push_back(CString(_T("default")));
	result.push_back(CString(_T("freetype:")));
	for (it = buf.begin(); it != buf.end(); it++) {
		ch = *(LPCTSTR)*it;
		if (ch >= 0x80) result.push_back(*it);
	}
	for (it = buf.begin(); it != buf.end(); it++) {
		ch = *(LPCTSTR)*it;
		if (ch < 0x80 && ch != '@') result.push_back(*it);
	}
	for (it = buf.begin(); it != buf.end(); it++) {
		ch = *(LPCTSTR)*it;
		if (ch < 0x80 && ch == '@') result.push_back(*it);
	}
}
CString EnumFontface::GetValueTitle(const CString &value)
{
	if (value == CString(_T("default"))) {
		return STRTABLE(IDS_DEFAULTFONT);
	}
	if (value.Left(9) == CString(_T("freetype:"))) {
		return STRTABLE(IDS_FREETYPEFONT);
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumFontface::GetValueDescription(const CString &value)
{
	if (value == CString(_T("default"))) {
		return STRTABLE(IDS_DEFAULTFONT_DESC);
	}
	if (value.Left(9) == CString(_T("freetype:"))) {
		CString desc;
		TCHAR *tmp = _tcsdup(value);
		TCHAR *pfn, *pidx;
		pfn = _tcstok(tmp + 9, _T("|"));
        if (pfn == NULL) {
            pfn = _T("");
            pidx = NULL;
        } else {
            pidx = _tcstok(NULL, _T("|"));
        }
        if (pidx == NULL) {
            pidx = _T("0");
        }
		desc.Format(IDS_FREETYPEFONT_DESC, pfn, pidx);
		free(tmp);
		return desc;
	}
	CString desc;
	desc.Format(STRTABLE(IDS_GDIFONT_DESC), (LPCTSTR) value);
	return desc;
}
bool EnumFontface::TranslateSelectedValue(CWnd *listWnd, CString &value)
{
	if (value == CString(_T("freetype:"))) {
		TCHAR curdir[MAXLINE];
		GetCurrentDirectory(MAXLINE, curdir);
		if (curdir[_tcslen(curdir) - 1] != '\\') {
			_tcscat(curdir, _T("\\"));
		}
		
		CString filters = STRTABLE(IDS_FREETYPEFONT_FILTER);

		CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR, filters, listWnd);

		bool ret = (fileDlg.DoModal() == IDOK);
		TCHAR *pathName = NULL;

		if (ret) {
			pathName = _tcsdup(fileDlg.GetPathName());

			TCHAR *rpath;
			if (_tcsncmp(pathName, curdir, _tcslen(curdir)) == 0) {
				rpath = pathName + _tcslen(curdir);
			} else {
				rpath = pathName;
			}

			value.Format(_T("freetype:%s|0"), rpath);
		}

		SetCurrentDirectory(curdir);
		free(pathName);
		return ret;
	}
	return true;
}
bool EnumFontface::IsValueEqual(const CString &lstval, const CString &selval)
{
	if (lstval.Left(9) == CString(_T("freetype:"))) {
		return selval.Left(9) == CString(_T("freetype:"));
	}
	return lstval == selval;
}
