#include "stdafx.h"
#include <d3d9.h>

ConfigDescItem *p3DAPIConfigItem;

struct D3DEnumeration {
	std::string d3d9cfg;
	std::vector<CString> displaymode, depthstencil, multisample;
};

static D3DEnumeration d3denum;

static bool D3DEnumerationWriter(FILE *fp)
{
	fprintf(fp, "d3denum %s\n", d3denum.d3d9cfg.c_str());
	return true;
}

static bool D3DEnumerationReader(FILE *fp)
{
	static wchar_t *wbuf = NULL;
	char buf[MAXLINE];
	char ch;
	std::vector<CString> *vec = NULL;
	std::set<std::vector<CString> *> vset;
	while (fscanf(fp, MAXLINEFMT "%c", buf, &ch) == 2) {
		if (!vec) {
			if (strcmp(buf, "displaymode") == 0) {
				vec = &d3denum.displaymode;
			} else if (strcmp(buf, "depthstencil") == 0) {
				vec = &d3denum.depthstencil;
			} else if (strcmp(buf, "multisample") == 0) {
				vec = &d3denum.multisample;
			} else {
				return false;
			}
			if (vset.find(vec) == vset.end()) {
				vset.insert(vec);
			} else {
				return false;
			}
		} else {
			vec->push_back(CString(cs2wcs_managed(buf, CP_UTF8, &wbuf)));
		}
		switch (ch) {
		case '\n': vec = NULL; break;
		case ' ': break;
		default: return false;
		}
	}
	return vset.size() == 3;
}

static bool ReloadD3DEnumeration(CWnd *fawnd, LPCTSTR d3d9cfg, bool backup)
{
	static char *buf = NULL;
	D3DEnumeration d3denum_bak = d3denum;
	d3denum.d3d9cfg = wcs2cs_managed(d3d9cfg, CP_UTF8, &buf);
	d3denum.displaymode.clear();
	d3denum.depthstencil.clear();
	d3denum.multisample.clear();
	if (!backup) {
		d3denum_bak = d3denum;
	}
	if (InvokeTempCommand(fawnd, D3DEnumerationWriter, D3DEnumerationReader)) {
		return true;
	} else {
		d3denum = d3denum_bak;
		return false;
	}
}

void FirstD3DEnumeration()
{
	if (!ReloadD3DEnumeration(GetPleaseWaitDlg(), *p3DAPIConfigItem->pvalue, false)) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_NOD3DENUM), STRTABLE(IDS_NOD3DENUM_TITLE), MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
	} else {
		if (FallbackConfigData(true) > 0) {
			if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_D3DENUM_ASKFALLBACK), STRTABLE(IDS_D3DENUM_ASKFALLBACK_TITLE), MB_ICONWARNING | MB_YESNO) == IDYES) {
				FallbackConfigData(false);
			}
		}
	}
}

void On3DAPIConfigReset(CPatchConfigDlg *dlg)
{
	dlg->SetTopMost(false);
	ShowPleaseWaitDlg(dlg, STRTABLE(IDS_WAITINGENUMD3D));
	if (!ReloadD3DEnumeration(GetPleaseWaitDlg(), *p3DAPIConfigItem->pvalue, false)) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_NOD3DENUM), STRTABLE(IDS_NOD3DENUM_TITLE), MB_ICONWARNING);
	}
	DestroyPleaseWaitDlg(dlg);
	dlg->SetTopMost(true);
	dlg->SetForegroundWindow();
}

bool On3DAPIConfigChange(CPatchConfigDlg *dlg, const CString &oldvalue, const CString &newvalue)
{
	dlg->SetTopMost(false);
	ShowPleaseWaitDlg(dlg, STRTABLE(IDS_WAITINGENUMD3D));
	bool success = ReloadD3DEnumeration(GetPleaseWaitDlg(), newvalue, true);
	if (!success) {
		GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_NO3DAPI), STRTABLE(IDS_NO3DAPI_TITLE), MB_ICONERROR);
	} else {
		if (FallbackConfigData(true) > 0) {
			if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_3DAPICHG_ASKFALLBACK), STRTABLE(IDS_3DAPICHG_ASKFALLBACK_TITLE), MB_ICONWARNING | MB_OKCANCEL) == IDOK) {
				FallbackConfigData(false);
			} else {
				success = false;
			}
		}
	}
	DestroyPleaseWaitDlg(dlg);
	dlg->SetTopMost(true);
	dlg->SetForegroundWindow();
	return success;
}

EnumDisplayMode EnumDisplayModeInstance;
void EnumDisplayMode::EnumConfigValues(std::vector<CString> &result)
{
	result.clear();
	result.push_back(CString(_T("current")));
	result.insert(result.end(), d3denum.displaymode.begin(), d3denum.displaymode.end());
	result.push_back(CString(_T("800x600")));
}
CString EnumDisplayMode::GetValueTitle(const CString &value)
{
	if (value == CString(_T("current"))) {
		return STRTABLE(IDS_AUTORESOLUTION);
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumDisplayMode::GetValueDescription(const CString &value)
{
	if (value == CString(_T("current"))) {
		return STRTABLE(IDS_AUTORESOLUTION_DESC);
	}
	CString desc;
	desc.Format(IDS_RESOLUTION_DESC, (LPCTSTR)value);
	return desc;
}


EnumDepthStencil EnumDepthStencilInstance;
void EnumDepthStencil::EnumConfigValues(std::vector<CString> &result)
{
	result.clear();
	result.push_back(CString(_T("auto")));
	result.push_back(CString(_T("16")));
	result.push_back(CString(_T("24")));
	result.insert(result.end(), d3denum.depthstencil.begin(), d3denum.depthstencil.end());
}
CString EnumDepthStencil::GetFallbackValue()
{
	return CString(_T("auto"));
}
CString EnumDepthStencil::GetValueTitle(const CString &value)
{
	if (value == CString(_T("auto"))) {
		return STRTABLE(IDS_AUTOSELECT);
	}
	if (value == CString(_T("16"))) {
		return STRTABLE(IDS_AUTOZBUF16);
	}
	if (value == CString(_T("24"))) {
		return STRTABLE(IDS_AUTOZBUF24);
	}
	int fmt;
	if (_stscanf(value, _T("-%d"), &fmt) == 1) {
		switch (fmt) {
		case D3DFMT_D16:
			return STRTABLE(IDS_ZBUF_D3DFMT_D16);
		case D3DFMT_D15S1:
			return STRTABLE(IDS_ZBUF_D3DFMT_D15S1);
		case D3DFMT_D24X8:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24X8);
		case D3DFMT_D24S8:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24S8);
		case D3DFMT_D24X4S4:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24X4S4);
		case D3DFMT_D32:
			return STRTABLE(IDS_ZBUF_D3DFMT_D32);
		}
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumDepthStencil::GetValueDescription(const CString &value)
{
	if (value == CString(_T("auto"))) {
		return STRTABLE(IDS_AUTOSELECT_DESC);
	}
	return ConfigDescOptionListEnum::GetValueDescription(value);
}


EnumMultisample EnumMultisampleInstance;
void EnumMultisample::EnumConfigValues(std::vector<CString> &result)
{
	result.clear();
	result.push_back(CString(_T("auto,auto")));
	result.push_back(CString(_T("0,0")));
	result.insert(result.end(), d3denum.multisample.begin(), d3denum.multisample.end());
}
CString EnumMultisample::GetFallbackValue()
{
	return CString(_T("0,0"));
}
CString EnumMultisample::GetValueTitle(const CString &value)
{
	if (value == CString(_T("auto,auto"))) {
		return STRTABLE(IDS_AUTOSELECT);
	}
	if (value == CString(_T("0,0"))) {
		return STRTABLE(IDS_MSAA_NONE);
	}
	int mtype, q;
	if (_stscanf(value, _T("%d,%d"), &mtype, &q) == 2) {
		CString str;
		if (mtype == D3DMULTISAMPLE_NONMASKABLE) {
			str.Format(IDS_MSAA_NONMASKABLE_FORMAT, q);
		} else {
			str.Format(IDS_MSAA_FORMAT, mtype, q);
		}
		return str;
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumMultisample::GetValueDescription(const CString &value)
{
	if (value == CString(_T("auto,auto"))) {
		return STRTABLE(IDS_AUTOSELECT_DESC);
	}
	if (value == CString(_T("0,0"))) {
		return STRTABLE(IDS_MSAA_NONE_DESC);
	}
	return ConfigDescOptionListEnum::GetValueDescription(value);
}
